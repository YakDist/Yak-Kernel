#include <limine-generic/request.h>
#include <limine.h>

extern "C" {
[[gnu::used, gnu::section(".limine_requests_start")]]
static volatile uint64_t limine_requests_start[] = LIMINE_REQUESTS_START_MARKER;

[[gnu::used, gnu::section(".limine_requests_end")]]
static volatile uint64_t limine_requests_end[] = LIMINE_REQUESTS_END_MARKER;

LIMINE_REQ static volatile uint64_t limine_base_revision[] =
    LIMINE_BASE_REVISION(6);
}
