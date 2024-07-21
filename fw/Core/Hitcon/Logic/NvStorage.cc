#include <Logic/NvStorage.h>
#include <Logic/crc32.h>
#include <Service/FlashService.h>
#include <Service/Sched/Checks.h>
#include <Service/Sched/Scheduler.h>
#include <main.h>
#include <string.h>

using hitcon::service::sched::my_assert;
using hitcon::service::sched::scheduler;

namespace hitcon {
NvStorage g_nv_storage;

NvStorage::NvStorage()
    : routine_task(800, (callback_t)&NvStorage::Routine, this, 100) {}

void NvStorage::Init() {
  int32_t newest_version = -1;
  my_assert(!g_flash_service.IsBusy());
  for (size_t i = 0; i < FLASH_PAGE_COUNT; i++) {
    uint8_t* page_data =
        reinterpret_cast<uint8_t*>(g_flash_service.GetPagePointer(i));
    nv_storage_content* page_content =
        reinterpret_cast<nv_storage_content*>(page_data);
    if (page_content->version > newest_version &&
        crc32(page_data + sizeof(uint32_t),
              sizeof(nv_storage_content) - sizeof(uint32_t)) ==
            page_content->checksum) {
      newest_version = page_content->version;
      memcpy(&content_, page_content, sizeof(nv_storage_content));
      next_available_page = (i + 1) % FLASH_PAGE_COUNT;
    }
  }
  if (newest_version == -1) {
    memset(&content_, 0, sizeof(nv_storage_content));
    content_.version = 1;
    content_.checksum = 0;
    storage_dirty_ = true;
    force_flush = true;
  }
  storage_valid_ = true;

  scheduler.Queue(&routine_task, nullptr);
  scheduler.EnablePeriodic(&routine_task);
}

void NvStorage::ForceFlushInternal() {
  if (!storage_dirty_) return;
  if (g_flash_service.IsBusy()) return;

  memcpy(&write_buffer_, &content_, sizeof(nv_storage_content));
  write_buffer_.checksum =
      crc32(reinterpret_cast<uint8_t*>(&write_buffer_) + sizeof(uint32_t),
            sizeof(nv_storage_content) - sizeof(uint32_t));
  bool ret = g_flash_service.ProgramPage(
      next_available_page, reinterpret_cast<uint32_t*>(&write_buffer_),
      sizeof(nv_storage_content));
  if (ret) {
    storage_dirty_ = false;
    next_available_page = (next_available_page + 1) %
                          FLASH_PAGE_COUNT;  // Increment for the next write
    last_flush_cycle = current_cycle;        // Record the current cycle
  }
}

void NvStorage::ForceFlush(callback_t on_done, void* callback_arg1) {
  force_flush = true;
  on_done_cb = on_done;
  on_done_cb_arg1 = callback_arg1;
}

void NvStorage::Routine(void* unused) {
  current_cycle++;

  if (on_done_cb && !force_flush && !g_flash_service.IsBusy()) {
    on_done_cb(on_done_cb_arg1, nullptr);
    on_done_cb = nullptr;
  }

  if ((force_flush || current_cycle - last_flush_cycle >= 300)) {
    ForceFlushInternal();
    force_flush = false;
  }
}

}  // namespace hitcon
