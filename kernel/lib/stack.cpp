extern "C" void __stack_chk_fail() {
  __builtin_trap();
}
