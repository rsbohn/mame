// license:BSD-3-Clause
// copyright-holders: Randall Bohn
/* Pluton/0 

A simple 65816 computer.

2019-03-03 Start here.

****/
#include "emu.h"
#include "cpu/g65816/g65816.h"

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
MACHINE_CONFIG_END

ROM_START( pzero )
  ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
  ROM_LOAD("rom000.bin", 0xf000, 0x1000, CRC(0ecb1b7a) SHA1(9057d32bba9be7f19f140818f29c17e6e24b83cd))

ROM_END

/* Driver */

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY                FULLNAME  FLAGS */
COMP( 2019, pzero,  0,      0,      pzero,    pzero,  pzero_state,  empty_init, "Bodge Industrial", "Pluton/0",  MACHINE_NO_SOUND_HW)