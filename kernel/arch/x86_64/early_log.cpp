#include <cstddef>
#include <x86_64/asm.h>
#include <yak/log.h>

#if CONFIG_EARLYFB
#include <flanterm.h>
#include <flanterm_backends/fb.h>
#include <limine-generic/request.h>
#include <limine.h>
#endif

namespace yak::arch {

#if CONFIG_EARLYFB
extern "C" {
LIMINE_REQ
volatile struct limine_framebuffer_request fb_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
    .response = nullptr,
};
}

flanterm_context *early_context;
#endif

constexpr int COM1 = 0x3F8;

static void early_putc(const char c) {
  while (!(asm_inb(COM1 + 5) & 0x20))
    asm volatile("pause");

  asm_outb(COM1, c);

#if CONFIG_EARLYFB
  if (early_context)
    flanterm_write(early_context, &c, 1);
#endif
}

void early_puts(const char *buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (buf[i] == '\n') {
      early_putc('\r');
    }
    early_putc(buf[i]);
  }
}

void early_output_init() {
#if CONFIG_EARLYFB
  struct limine_framebuffer_response *res = fb_request.response;
  if (0 == res->framebuffer_count)
    return;

  struct limine_framebuffer *fb = res->framebuffers[0];
  early_context = flanterm_fb_init(
      nullptr, nullptr, (uint32_t *) fb->address, fb->width, fb->height,
      fb->pitch, fb->red_mask_size, fb->red_mask_shift, fb->green_mask_size,
      fb->green_mask_shift, fb->blue_mask_size, fb->blue_mask_shift, NULL, NULL,
      NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 1, 0, 0, 0,
      FLANTERM_FB_ROTATE_0);
#endif
}

} // namespace yak::arch
