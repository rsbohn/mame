// license:BSD-3-Clause
// copyright-holders: Randall Bohn
/* Pluton/0 

A simple 65816 computer.

2019-03-03 Start here.

****/
#include "emu.h"
#include "cpu/g65816/g65816.h"
#include "machine/mos6551.h"
#include "machine/keyboard.h"
#include "bus/rs232/rs232.h"

#define KEYBOARD_TAG "keyboard"

class pzero_state: public driver_device
{
public:
  pzero_state(const machine_config &mconfig,
	device_type type, const char *tag)

    : driver_device(mconfig, type, tag)
    , m_maincpu(*this, "maincpu")

  {}

  void pzero(machine_config &config);
  void pzero_mem(address_map &map);
private:
  virtual void machine_reset() override;
  required_device<cpu_device> m_maincpu;
};

void pzero_state::pzero_mem(address_map &map)
{
  map.unmap_value_high();
  map(0x0000, 0xdfff).ram();
  map(0xe120, 0xe123).rw("port0",
	FUNC(mos6551_device::read), FUNC(mos6551_device::write));
  map(0xe130, 0xe133).rw("port1",
	FUNC(mos6551_device::read), FUNC(mos6551_device::write));
  map(0xf000, 0xffff).rom();
}

static INPUT_PORTS_START( pzero )
INPUT_PORTS_END

void pzero_state::machine_reset()
{
}

/************* CONFIG **************/
MACHINE_CONFIG_START(pzero_state::pzero)
  MCFG_DEVICE_ADD("maincpu", G65816, XTAL(4'000'000))
  MCFG_DEVICE_PROGRAM_MAP(pzero_mem)
  
  mos6551_device &port0(MOS6551(config, "port0", 0));
  mos6551_device &port1(MOS6551(config, "port1", 0));

  port0.set_xtal(XTAL(1'843'200));
  port1.set_xtal(XTAL(1'843'200));

  port0.txd_handler().set("eia0", FUNC(rs232_port_device::write_txd));
  port0.rts_handler().set("eia0", FUNC(rs232_port_device::write_rts));

  rs232_port_device &eia0(RS232_PORT(config, "eia0", default_rs232_devices, "terminal"));
  eia0.rxd_handler().set("port0", FUNC(mos6551_device::write_rxd));
  eia0.cts_handler().set("port0", FUNC(mos6551_device::write_cts));
  //eia0.dcd_handler().set("port0", FUNC(mos6551_device::write_dcd));
  //eia0.dsr_handler().set("port0", FUNC(mos6551_device::write_dsr));


MACHINE_CONFIG_END

ROM_START( pzero )
  ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
  ROM_LOAD("rom001.bin", 0xf000, 0x1000, CRC(92609fd5) SHA1(9057d32bba9be7f19f140818f29c17e6e24b83cd))

ROM_END

/* Driver */

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY                FULLNAME  FLAGS */
COMP( 2019, pzero,  0,      0,      pzero,    pzero,  pzero_state,  empty_init, "Bodge Industrial", "Pluton/0",  MACHINE_NO_SOUND_HW)
