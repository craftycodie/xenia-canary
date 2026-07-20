/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include <set>

#include "xenia/base/debugging.h"
#include "xenia/base/logging.h"
#include "xenia/emulator.h"
#include "xenia/kernel/kernel_state.h"
#include "xenia/kernel/util/shim_utils.h"
#include "xenia/kernel/xboxkrnl/xboxkrnl_private.h"
#include "xenia/kernel/xthread.h"
#include "xenia/ui/imgui_dialog.h"
#include "xenia/ui/imgui_drawer.h"
#include "xenia/ui/window.h"
#include "xenia/ui/windowed_app_context.h"

namespace xe {
namespace kernel {
namespace xboxkrnl {

void DbgBreakPoint_entry() { xe::debugging::Break(); }
DECLARE_XBOXKRNL_EXPORT2(DbgBreakPoint, kDebug, kStub, kImportant);

// https://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
typedef struct {
  xe::be<uint32_t> type;
  xe::be<uint32_t> name_ptr;
  xe::be<uint32_t> thread_id;
  xe::be<uint32_t> flags;
} X_THREADNAME_INFO;
static_assert_size(X_THREADNAME_INFO, 0x10);

void HandleSetThreadName(pointer_t<X_EXCEPTION_RECORD> record) {
  // SetThreadName. FFS.
  // https://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx

  // TODO(benvanik): check record->number_parameters to make sure it's a
  // correct size.
  auto thread_info =
      reinterpret_cast<X_THREADNAME_INFO*>(&record->exception_information[0]);

  assert_true(thread_info->type == 0x1000);

  if (!thread_info->name_ptr) {
    XELOGD("SetThreadName called with null name_ptr");
    return;
  }

  // 4D5307D6 (and its demo) has a bug where it ends up passing freed memory for
  // the name, so at the point of SetThreadName it's filled with junk.

  // TODO(gibbed): cvar for thread name encoding for conversion, some games use
  // SJIS and there's no way to automatically know this.
  auto name = std::string(
      kernel_memory()->TranslateVirtual<const char*>(thread_info->name_ptr));
  std::replace_if(
      name.begin(), name.end(), [](auto c) { return c < 32 || c > 127; }, '?');

  object_ref<XThread> thread;
  if (thread_info->thread_id == -1) {
    // Current thread.
    thread = retain_object(XThread::GetCurrentThread());
  } else {
    // Lookup thread by ID.
    thread = kernel_state()->GetThreadByID(thread_info->thread_id);
  }

  if (thread) {
    XELOGD("SetThreadName({}, {})", thread->thread_id(), name);
    thread->set_name(name);
  }

  // TODO(benvanik): unwinding required here?
}

typedef struct {
  xe::be<int32_t> mdisp;
  xe::be<int32_t> pdisp;
  xe::be<int32_t> vdisp;
} x_PMD;

typedef struct {
  xe::be<uint32_t> properties;
  xe::be<uint32_t> type_ptr;
  x_PMD this_displacement;
  xe::be<int32_t> size_or_offset;
  xe::be<uint32_t> copy_function_ptr;
} x_s__CatchableType;

typedef struct {
  xe::be<int32_t> number_catchable_types;
  xe::be<uint32_t> catchable_type_ptrs[1];
} x_s__CatchableTypeArray;

typedef struct {
  xe::be<uint32_t> attributes;
  xe::be<uint32_t> unwind_ptr;
  xe::be<uint32_t> forward_compat_ptr;
  xe::be<uint32_t> catchable_type_array_ptr;
} x_s__ThrowInfo;

void HandleCppException(pointer_t<X_EXCEPTION_RECORD> record) {
  // C++ exception.
  // https://blogs.msdn.com/b/oldnewthing/archive/2010/07/30/10044061.aspx
  // http://www.drdobbs.com/visual-c-exception-handling-instrumentat/184416600
  // http://www.openrce.org/articles/full_view/21

  assert_true(record->number_parameters == 3);
  assert_true(record->exception_information[0] == 0x19930520);

  auto thrown_ptr = record->exception_information[1];
  auto thrown = kernel_memory()->TranslateVirtual(thrown_ptr);
  auto vftable_ptr = *reinterpret_cast<xe::be<uint32_t>*>(thrown);

  auto throw_info_ptr = record->exception_information[2];
  auto throw_info =
      kernel_memory()->TranslateVirtual<x_s__ThrowInfo*>(throw_info_ptr);
  auto catchable_types =
      kernel_memory()->TranslateVirtual<x_s__CatchableTypeArray*>(
          throw_info->catchable_type_array_ptr);

  // xe::debugging::Break();
  XELOGE("Guest attempted to throw a C++ exception!");
}

void RtlRaiseException_entry(pointer_t<X_EXCEPTION_RECORD> record,
                             const ppc_context_t& context) {
  switch (record->code) {
    case 0x406D1388: {
      HandleSetThreadName(record);
      return;
    }
    case 0xE06D7363: {
      HandleCppException(record);
      return;
    }
  }

  // TODO(benvanik): unwinding.
  // This is going to suck.
  // xe::debugging::Break();

  // RtlRaiseException definitely wasn't a noreturn function, we can return
  // safe-ish. Log the guest LR plus a back-chain stack walk so the assert that
  // raised it can be located. The game's assert strings are stripped in
  // release_internal (message/file = "UNKNOWN", line = -1), so the only way to
  // identify a specific assert is by its guest call site. PPC back-chain rule
  // (derived from the raise wrapper's prologue): the return address into the
  // function that owns frame [sp .. *(sp)] is stored at *( *(sp) - 8 ).
  // Only read guest memory whose page is actually committed + readable, or we
  // fault the host when following arbitrary/stale pointers off the stack.
  auto guest_readable = [](uint32_t addr) -> bool {
    if (addr < 0x1000 || addr >= 0xC0000000) return false;
    auto* heap = kernel_memory()->LookupHeap(addr);
    if (!heap) return false;
    uint32_t protect = 0;
    if (!heap->QueryProtect(addr, &protect)) return false;
    return (protect & kMemoryProtectRead) != 0;
  };
  auto read_guest_u32 = [&](uint32_t addr) -> uint32_t {
    if (!guest_readable(addr) || !guest_readable(addr + 3)) return 0;
    auto* p = kernel_memory()->TranslateVirtual<xe::be<uint32_t>*>(addr);
    return p ? uint32_t(*p) : 0;
  };

  // Investment-apply cast crash (Destiny tiger): the assert fires deep inside
  // the type-15 event apply while casting the nested object to the expected
  // runtime type off_839E6E68 (*0x83FAAE70 == 0x8080386D). Return addresses in
  // these ranges up the back-chain flag that specific crash so we can dump the
  // response payload + nested type-tag WITHOUT spamming every unrelated assert.
  auto in_range = [](uint32_t a, uint32_t lo, uint32_t hi) {
    return a >= lo && a < hi;
  };
  auto is_invest_apply = [&](uint32_t ret) {
    return in_range(ret, 0x82BABB18, 0x82BABCD8) ||  // sub_82BABB18/BF8 apply
           in_range(ret, 0x83859D80, 0x83859F20) ||  // sub_83859D80 cast-apply
           in_range(ret, 0x828B2B30, 0x828B2C00) ||  // sub_828B2B30 cast
           in_range(ret, 0x832466F8, 0x83246900);    // sub_832466F8 apply-3
  };

  std::string frames;
  bool invest_crash = false;
  uint32_t sp = uint32_t(context->r[1]);
  for (int i = 0; i < 16 && sp; ++i) {
    uint32_t caller_sp = read_guest_u32(sp);
    if (caller_sp <= sp) break;  // stack grows down; back chain must increase
    uint32_t ret = read_guest_u32(caller_sp - 8);
    if (ret) {
      frames += fmt::format(" {:08X}", ret);
      if (is_invest_apply(ret)) invest_crash = true;
    }
    sp = caller_sp;
  }

  XELOGE(
      "Guest attempted to trigger a breakpoint! code={:08X} "
      "exception_address={:08X} flags={:08X} lr={:08X} nparams={} "
      "info0={:08X} info1={:08X} info2={:08X} guest_stack:{}",
      uint32_t(record->code), uint32_t(record->exception_address),
      uint32_t(record->exception_flags), uint32_t(context->lr),
      uint32_t(record->number_parameters),
      uint32_t(record->exception_information[0]),
      uint32_t(record->exception_information[1]),
      uint32_t(record->exception_information[2]), frames);

  if (!invest_crash) {
    return;
  }

  // --- Investment-apply crash context dump (one-shot at the assert) ---------
  auto is_guest_ptr = [](uint32_t a) {
    return a >= 0x10000000 && a < 0xC0000000 && (a & 3) == 0;
  };
  auto hex_guest = [&](uint32_t addr, uint32_t len) -> std::string {
    std::string s;
    for (uint32_t o = 0; o < len; o += 4) {
      s += fmt::format(" {:08X}", read_guest_u32(addr + o));
    }
    return s;
  };

  // Full GPR file at the assert. r3/r4 at the failed cast were the object
  // type-tag and the expected type (0x8080386D) respectively before the assert
  // plumbing ran; the rest help locate the live frames.
  std::string gprs;
  for (int i = 0; i < 32; ++i) {
    gprs += fmt::format(" r{}={:08X}", i, uint32_t(context->r[i]));
  }
  XELOGE("INVEST_CRASH gprs:{}", gprs);

  // Expected nested type chain for cross-check (off_839E6E68 -> descriptor ->
  // first entry hash, should be 0x8080386D).
  uint32_t exp_ptr = read_guest_u32(0x839E6E68);
  XELOGE("INVEST_CRASH expected_type *(0x839E6E68)={:08X} *that={:08X}", exp_ptr,
         read_guest_u32(exp_ptr));

  // Dump a window of the crashing thread's stack, then dereference every guest
  // pointer found in it (dedup, capped). The type-15 event's nested payload
  // pointer (a3[2]) + length (a3[3]) live in the apply frame; following the
  // pointer reveals the object's leading type-tag (why the cast failed) and the
  // response bytes we must synthesize.
  uint32_t base_sp = uint32_t(context->r[1]);
  const uint32_t kStackWin = 0x800;
  for (uint32_t o = 0; o < kStackWin; o += 32) {
    XELOGE("INVEST_CRASH stack[{:08X}]:{}", base_sp + o,
           hex_guest(base_sp + o, 32));
  }

  std::set<uint32_t> seen;
  int dumped = 0;
  for (uint32_t o = 0; o < kStackWin && dumped < 48; o += 4) {
    uint32_t v = read_guest_u32(base_sp + o);
    if (!is_guest_ptr(v) || seen.count(v)) continue;
    seen.insert(v);
    XELOGE("INVEST_CRASH deref[{:08X}](@sp+{:X}):{}", v, o, hex_guest(v, 48));
    ++dumped;
  }

  // The type-15 event pointer (sub_82BABBF8 `a3`: a3[1]=obj1 data,
  // a3[2]=nested-object ptr, a3[3]=nested len) is used across the whole apply,
  // so it is most likely held in a nonvolatile GPR at the deep assert rather
  // than on the stack. Deref every guest-pointer GPR with a wide window: this
  // surfaces the event body (whose header/request_parameters carry the
  // originating investment network-id that identifies WHICH message's response
  // must supply the nested 0x8080386D queuez object), and following a3[2]
  // reveals the nested object's leading type-tag.
  for (int i = 0; i < 32; ++i) {
    uint32_t v = uint32_t(context->r[i]);
    if (!is_guest_ptr(v) || seen.count(v)) continue;
    seen.insert(v);
    XELOGE("INVEST_CRASH gpr_deref r{}[{:08X}]:{}", i, v, hex_guest(v, 96));
  }
}
DECLARE_XBOXKRNL_EXPORT2(RtlRaiseException, kDebug, kStub, kImportant);

void KeBugCheckEx_entry(dword_t code, dword_t param1, dword_t param2,
                        dword_t param3, dword_t param4) {
  auto msg =
      fmt::format("*** STOP: 0x{:08X} (0x{:08X}, 0x{:08X}, 0x{:08X}, 0x{:08X})",
                  static_cast<uint32_t>(code), static_cast<uint32_t>(param1),
                  static_cast<uint32_t>(param2), static_cast<uint32_t>(param3),
                  static_cast<uint32_t>(param4));
  XELOGE("{}", msg);
  fflush(stdout);

  if (xe::debugging::IsDebuggerAttached()) {
    xe::debugging::Break();
  }

  // Show crash dialog and suspend the guest thread instead of killing the
  // host process.
  auto current_thread = kernel::XThread::GetCurrentThread();
  const auto* emulator = kernel_state()->emulator();
  auto* display_window = emulator->display_window();
  auto* imgui_drawer = emulator->imgui_drawer();
  if (display_window && imgui_drawer) {
    auto dlg_msg = fmt::format(
        "The guest kernel has crashed (KeBugCheck).\n\n{}\n\n"
        "The faulting thread has been suspended.",
        msg);
    display_window->app_context().CallInUIThreadSynchronous(
        [imgui_drawer, &dlg_msg]() {
          xe::ui::ImGuiDialog::ShowMessageBox(imgui_drawer,
                                              "Guest Kernel Crash", dlg_msg);
        });
  }

  if (current_thread) {
    current_thread->Suspend(nullptr);
  }
}
DECLARE_XBOXKRNL_EXPORT2(KeBugCheckEx, kDebug, kStub, kImportant);

void KeBugCheck_entry(dword_t code) { KeBugCheckEx_entry(code, 0, 0, 0, 0); }
DECLARE_XBOXKRNL_EXPORT2(KeBugCheck, kDebug, kImplemented, kImportant);

}  // namespace xboxkrnl
}  // namespace kernel
}  // namespace xe

DECLARE_XBOXKRNL_EMPTY_REGISTER_EXPORTS(Debug);
