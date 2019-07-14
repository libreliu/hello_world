#include <iostream>
#include <hw/gpio.hpp>
#include <hw/screen.hpp>
#include <os>
#include <fs/disk.hpp>
#include <arch/aarch64/uart.h>
#include <hw/emmc.hpp>
#include <hal/machine.hpp>
#include <fs/vfs.hpp>

void read_file() {
  hw::Rpi_Emmc& emmc = hw::Rpi_Emmc::get();
  static std::shared_ptr<fs::Disk> disk = std::make_shared<fs::Disk>(emmc);
  disk->init_fs(disk->MBR, [](fs::error_t err, fs::File_system& fs) {
    printf("Initializing Disk...\n");
    return;
  });
  
  // VFS Related
  // However fs:: (vfs) seems to be buggy, use disk->fs(). the FAT handler instead
  fs::mount({"/", disk->name()}, disk->dev(), "anything else");
  fs::print_tree();

  disk->fs().print_subtree("/");

  auto p = fs::Path({"/", disk->name(), "/osh-test"});
  std::cout << "Path: " << p.to_string() << std::endl;
  auto file = disk->fs().stat("/osh-test/hello.txt");
  std::cout << file.read();

}

int main(){

  std::cout << "Hello world\n";
  hw::GPIO g;
  volatile __uint8_t led = (volatile __uint8_t) 29;
  volatile __uint32_t *timer_addr = (volatile __uint32_t *) 0x3F003004;
  g.gpio_func_select(led, (__uint8_t)1);
  int i;
  __uint32_t prev_time = g.read_peri(timer_addr);
  
  // Add Screen Support
  screen_init();
  os::add_stdout([](const char *str, size_t len) -> void {
    char *p = (char *)malloc(len + 1);
    for (size_t i = 0; i < len; i++)
      p[i] = str[i];
    p[len] = '\0';
    screen_print(p);
    free(p);
  });

  printf("This can be both in UART and Framebuffer!\n");
  // blink 10 times.
  for (int i = 0; i < 10; i++)
  {
      printf("LED on!\n");
      g.gpio_set(led);
      // /*for (;;)
      // {
      //     __uint32_t curr_time = g.read_peri(timer_addr);
      //     if (curr_time - prev_time >= 0x000f4240)
      //     {
      //         prev_time = curr_time;
      //         break;
      //     }
      // }
      g.gpio_clr(led);
      printf("LED off!\n");
  }
  // printf("Test gpio delay");
  // throw "WTF";
  read_file();
}
