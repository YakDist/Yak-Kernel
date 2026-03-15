#pragma once

namespace yak::arch {

enum class GdtIndex : int {
  Null = 0,
  KernelCode,
  KernelData,
  UserSyscall,
  UserData,
  UserCode,
  Tss,
};

constexpr int CPL_KERNEL = 0b00;
constexpr int CPL_USER = 0b11;

constexpr int make_sel(GdtIndex idx, int cpl) {
  return (static_cast<int>(idx) << 3) | cpl;
}

constexpr int GDT_SEL_NULL = make_sel(GdtIndex::Null, CPL_KERNEL);
constexpr int GDT_SEL_KERNEL_CODE = make_sel(GdtIndex::KernelCode, CPL_KERNEL);
constexpr int GDT_SEL_KERNEL_DATA = make_sel(GdtIndex::KernelData, CPL_KERNEL);
constexpr int GDT_SEL_USER_SYSCALL = make_sel(GdtIndex::UserSyscall, CPL_USER);
constexpr int GDT_SEL_USER_DATA = make_sel(GdtIndex::UserData, CPL_USER);
constexpr int GDT_SEL_USER_CODE = make_sel(GdtIndex::UserCode, CPL_USER);
constexpr int GDT_SEL_TSS = make_sel(GdtIndex::Tss, CPL_KERNEL);

void gdt_init();
void gdt_reload();
} // namespace yak::arch
