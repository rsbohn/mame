// license:BSD-3-Clause
// copyright-holders: Randall Bohn
/* Pluton/0 

A simple 65816 computer.

2019-03-11 Add a 6522 VIA chip.
	Add 32K nvram (./nvram/pzero/...)
2019-03-03 Start here.

****/
#include "emu.h"
#include "cpu/g65816/g65816.h"
#include "cpu/m6502/m6502.h"
#include "video/mc6845.h"
#include "machine/mos6551.h"
#include "machine/6522via.h"
#include "machine/keyboard.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "bus/rs232/rs232.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class pzero_state: public driver_device
{
public:
  pzero_state(const machine_config &mconfig,
	device_type type, const char *tag)

    : driver_device(mconfig, type, tag)
    , m_maincpu(*this, "maincpu")
    , m_via0(*this, "via0")
    , m_port0(*this, "port0")
    , m_port1(*this, "port1")
    , m_p_videoram(*this, "videoram")
    , m_p_chargen(*this, "chargen")
    , m_palette(*this, "palette") // why? idk

  {}

  MC6845_UPDATE_ROW(scanline);
  void pzero(machine_config &config);
  void pzero_mem(address_map &map);
  void key_in(u8 data);

private:
  virtual void machine_reset() override;
  required_device<cpu_device> m_maincpu;
  required_device<via6522_device> m_via0;
  required_device<mos6551_device> m_port0;
  required_device<mos6551_device> m_port1;
  required_shared_ptr<uint8_t> m_p_videoram;
  required_shared_ptr<u8> m_p_chargen; // was _region_ -- put it in a cart?
  required_device<palette_device> m_palette;
};

void pzero_state::pzero_mem(address_map &map)
{
  map.unmap_value_high();
  map(0x0000, 0x07ff).ram();
  map(0x0800,0x080f).rw("via0",
	FUNC(via6522_device::read), FUNC(via6522_device::write));
  map(0x0810, 0x081f).rw("port0",
	FUNC(mos6551_device::read), FUNC(mos6551_device::write));
  map(0x0820, 0x082f).rw("port1",
	FUNC(mos6551_device::read), FUNC(mos6551_device::write));
  map(0x0840, 0x0840).w("video", FUNC(mc6845_device::address_w));
  map(0x0841, 0x0841).rw("video", 
	FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
  map(0x0850, 0x0850).w("audio0", FUNC(ay8910_device::address_w));
  map(0x0851, 0x0851).rw("audio0",
	FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
  map(0x1000,0x7fff).ram();
  map(0x8000,0xefff).ram();  // RAM for now...
  map(0xf000, 0xffff).rom();

  map(0x10000, 0x17fff).ram().share("block01");
  map(0x18000, 0x18fff).ram().share("videoram"); // 4KB gives 4x1KB buffers
  map(0x19000, 0x19fff).ram().share("chargen"); // 4KB
  
}

/*
static const gfx_layout pzero_charla =
{
  8,8,256,1,
  {0},
  {0,1,2,3,4,5,6,7},
  {0*1,1*8,2*8,3*8,4*8,5*8,6*8,7*8},
  8*16
};
*/

MC6845_UPDATE_ROW( pzero_state::scanline )
{
        const rgb_t *palette = m_palette->palette()->entry_list_raw();
        uint8_t chr,gfx,inv;
        uint16_t mem,x;
        uint32_t *p = &bitmap.pix32(y);

        for (x = 0; x < x_count; x++)
        {
                inv = (x == cursor_x) ? 0xff : 0;
                mem = (ma + x) & 0x7ff;
                chr = m_p_videoram[mem];

                /* get pattern of pixels for that character scanline */
                gfx = m_p_chargen[(chr<<4) | (ra & 0x0f)] ^ inv;

                /* Display a scanline of a character */
                *p++ = palette[BIT(gfx, 7)];
                *p++ = palette[BIT(gfx, 6)];
                *p++ = palette[BIT(gfx, 5)];
                *p++ = palette[BIT(gfx, 4)];
                *p++ = palette[BIT(gfx, 3)];
                *p++ = palette[BIT(gfx, 2)];
                *p++ = palette[BIT(gfx, 1)];
                *p++ = palette[BIT(gfx, 0)];
        }
}

//////////////////
static INPUT_PORTS_START( pzero )
INPUT_PORTS_END

void pzero_state::key_in(u8 data)
{
  if (data)
  {
    m_via0->write_pa0(BIT(data, 0));
    m_via0->write_pa1(BIT(data, 1));
    m_via0->write_pa2(BIT(data, 2));
    m_via0->write_pa3(BIT(data, 3));
    m_via0->write_pa4(BIT(data, 4));
    m_via0->write_pa5(BIT(data, 5));
    m_via0->write_pa6(BIT(data, 6));
    m_via0->write_pa7(BIT(data, 7));
    m_via0->write_ca1(1);
    m_via0->write_ca1(0);
  }
}

void pzero_state::machine_reset()
{
}


/************* CONFIG **************/
MACHINE_CONFIG_START(pzero_state::pzero)
  MCFG_DEVICE_ADD("maincpu", G65816, XTAL(4'000'000))
  MCFG_DEVICE_PROGRAM_MAP(pzero_mem)
  NVRAM(config, "block01", nvram_device::DEFAULT_ALL_0);

  /** no semicolon after these macros **/
  MCFG_SCREEN_ADD("screen", RASTER)
  MCFG_SCREEN_REFRESH_RATE(60)
  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))  // wha?
  MCFG_SCREEN_SIZE(640,200)
  MCFG_SCREEN_VISIBLE_AREA(0,639,0,199)
  MCFG_SCREEN_UPDATE_DEVICE("video", mc6845_device, screen_update)

  PALETTE(config, "palette", palette_device::MONOCHROME);

  mc6845_device &video(MC6845(config, "video", XTAL(16'000'000)/8));
  video.set_screen("screen");
  video.set_show_border_area(false);
  video.set_char_width(8);
  video.set_update_row_callback(FUNC(pzero_state::scanline), this);

  
  VIA6522(config, m_via0, XTAL(4'000'000)/4);
  m_via0->irq_handler().set_inputline(m_maincpu, G65816_LINE_IRQ);

  mos6551_device &port0(MOS6551(config, m_port0, 0));
  mos6551_device &port1(MOS6551(config, m_port1, 0));

  port0.set_xtal(XTAL(1'843'200));
  port1.set_xtal(XTAL(1'843'200));

  port0.txd_handler().set("eia0", FUNC(rs232_port_device::write_txd));
  port0.rts_handler().set("eia0", FUNC(rs232_port_device::write_rts));
  port1.txd_handler().set("eia1", FUNC(rs232_port_device::write_txd));
  port1.rts_handler().set("eia1", FUNC(rs232_port_device::write_rts));

  rs232_port_device &eia0(RS232_PORT(config, "eia0", default_rs232_devices, nullptr));
  eia0.rxd_handler().set(m_port0, FUNC(mos6551_device::write_rxd));
  eia0.cts_handler().set(m_port0, FUNC(mos6551_device::write_cts));
  rs232_port_device &eia1(RS232_PORT(config, "eia1", default_rs232_devices, nullptr));
  eia1.rxd_handler().set(m_port1, FUNC(mos6551_device::write_rxd));
  eia1.cts_handler().set(m_port1, FUNC(mos6551_device::write_cts));

  SPEAKER(config, "mono").front_center();
  ay8910_device &audio0(AY8910(config, "audio0", XTAL(4'000'000)));
  audio0.set_flags(AY8910_SINGLE_OUTPUT);
  audio0.add_route(ALL_OUTPUTS, "mono", 0.50);

  generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
  keyboard.set_keyboard_callback(FUNC(pzero_state::key_in));

MACHINE_CONFIG_END

ROM_START( pzero )
  ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
  ROM_LOAD("monitor.rom", 0xf000, 0x1000, CRC(f570e682) SHA1(3d7474c500dd236a3adc80aad96d16832bda86fe))
ROM_END

/* Driver */

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY                FULLNAME  FLAGS */
COMP( 2019, pzero,  0,      0,      pzero,  pzero,  pzero_state, empty_init, "Bodge Industrial", "Pluton/0",  0)
