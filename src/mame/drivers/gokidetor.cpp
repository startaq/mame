// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Gokidetor  (c) 1992 Taito

    Mechanical whack-a-mole with a dot-matrix LED screen.

***************************************************************************/

#include "emu.h"
#include "speaker.h"
#include "cpu/z80/z80.h"
//#include "machine/m66240.h"
#include "machine/te7750.h"
#include "sound/2203intf.h"
#include "sound/okim6295.h"
#include "audio/taitosnd.h"

class gokidetor_state : public driver_device
{
public:
	gokidetor_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void gokidetor(machine_config &config);

private:
	void out4_w(uint8_t data);
	void out5_w(uint8_t data);
	void out6_w(uint8_t data);
	void out7_w(uint8_t data);
	void out8_w(uint8_t data);
	void out9_w(uint8_t data);
	void ym_porta_w(uint8_t data);

	void main_map(address_map &map);
	void sound_map(address_map &map);

	virtual void machine_start() override;

	required_device<cpu_device> m_maincpu;
};


void gokidetor_state::machine_start()
{
}


void gokidetor_state::out4_w(uint8_t data)
{
	logerror("Writing %02X to TE7750 port 4\n", data);
}

void gokidetor_state::out5_w(uint8_t data)
{
	logerror("Writing %02X to TE7750 port 5\n", data);
}

void gokidetor_state::out6_w(uint8_t data)
{
	logerror("Writing %02X to TE7750 port 6\n", data);
}

void gokidetor_state::out7_w(uint8_t data)
{
	logerror("Writing %02X to TE7750 port 7\n", data);
}

void gokidetor_state::out8_w(uint8_t data)
{
	logerror("Writing %02X to TE7750 port 8\n", data & 0x3f);
}

void gokidetor_state::out9_w(uint8_t data)
{
	logerror("Writing %02X to TE7750 port 9\n", data);
}

void gokidetor_state::ym_porta_w(uint8_t data)
{
	if (data != 0x40)
		logerror("Writing %02X to YM2203 port A\n", data);
}


void gokidetor_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).rom(); // probably banked
	map(0xa000, 0xbfff).ram();
	// c000-c??? = ?
	// c200-c??? = ?
	// d000 = ?output
	// d001 = ?output
	// d101 = ?output
	// d1c0 = ?output
	map(0xd800, 0xd80f).rw("te7750", FUNC(te7750_device::read), FUNC(te7750_device::write));
	//map(0xda00, 0xda01).w("pwm", FUNC(m66240_device::write));
	// de00 ?input
	// df00 ?input
	map(0xe000, 0xe003).nopr(); // ?input
	map(0xf000, 0xf000).w("ciu", FUNC(pc060ha_device::master_port_w));
	map(0xf001, 0xf001).rw("ciu", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
	// f600 ?output
	// f700 ?output
}


void gokidetor_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram();
	map(0x9000, 0x9001).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xa000, 0xa000).w("ciu", FUNC(pc060ha_device::slave_port_w));
	map(0xa001, 0xa001).rw("ciu", FUNC(pc060ha_device::slave_comm_r), FUNC(pc060ha_device::slave_comm_w));
	map(0xb000, 0xb000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


void gokidetor_state::gokidetor(machine_config &config)
{
	Z80(config, m_maincpu, XTAL(16'000'000) / 4); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &gokidetor_state::main_map);
	// IRQ from ???
	// NMI related to E002 input and TE7750 port 7

	te7750_device &te7750(TE7750(config, "te7750"));
	te7750.ios_cb().set_constant(3);
	te7750.in_port1_cb().set_ioport("IN1");
	te7750.in_port2_cb().set_ioport("IN2");
	te7750.in_port3_cb().set_ioport("IN3");
	te7750.out_port4_cb().set(FUNC(gokidetor_state::out4_w));
	te7750.out_port5_cb().set(FUNC(gokidetor_state::out5_w));
	te7750.out_port6_cb().set(FUNC(gokidetor_state::out6_w));
	te7750.out_port7_cb().set(FUNC(gokidetor_state::out7_w));
	te7750.in_port8_cb().set_ioport("IN8");
	te7750.out_port8_cb().set(FUNC(gokidetor_state::out8_w));
	te7750.out_port9_cb().set(FUNC(gokidetor_state::out9_w));

	z80_device &soundcpu(Z80(config, "soundcpu", 4000000));
	soundcpu.set_addrmap(AS_PROGRAM, &gokidetor_state::sound_map);

	pc060ha_device &ciu(PC060HA(config, "ciu", 0));
	ciu.set_master_tag(m_maincpu);
	ciu.set_slave_tag("soundcpu");

	SPEAKER(config, "mono").front_center();

	ym2203_device &ymsnd(YM2203(config, "ymsnd", 3000000));
	ymsnd.irq_handler().set_inputline("soundcpu", 0);
	ymsnd.port_a_write_callback().set(FUNC(gokidetor_state::ym_porta_w));
	ymsnd.add_route(0, "mono", 0.25);
	ymsnd.add_route(1, "mono", 0.25);
	ymsnd.add_route(2, "mono", 0.25);
	ymsnd.add_route(3, "mono", 0.80);

	OKIM6295(config, "oki", 1056000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.50); // clock frequency & pin 7 not verified
}

INPUT_PORTS_START( gokidetor )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("IN8")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

ROM_START( gokidetor )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ic2", 0, 0x20000, CRC(f9dbca28) SHA1(b2f6d6b66bfa5e5ca7c26a0709f7136bf9e1a42e) )
	// 8000-FFFF appears to be graphics; 10000-1FFFF is unused

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ic34", 0, 0x10000, CRC(fd06305d) SHA1(7889f0c360650bfd0fe593c522685a978879bfee) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "ic87", 0, 0x40000, CRC(adb9fd16) SHA1(59aae5ac26cae30b961b30e17fd494094aa7aa15) )

	ROM_REGION( 0x8000, "pals", 0 ) // unprotected
	ROM_LOAD( "d33-01.pal16l8a.ic20.jed", 0x0000, 0xb01, CRC(7d614a54) SHA1(e79b6ac99f46ff69ea0565e89d6807667c9e0547) )
	ROM_LOAD( "d33-02.pal16l8b.ic62.jed", 0x1000, 0xb01, CRC(6653423f) SHA1(ac93acccf649b97a3252aedf398310be90fe5d2d) )
	ROM_LOAD( "d33-03.pal16l8a.ic70.jed", 0x2000, 0xb01, CRC(abfd9a01) SHA1(c76cc16842231c2f688ddd46a17f3a6df92a9580) )
	ROM_LOAD( "d33-04.pal16l8b.ic81.jed", 0x3000, 0xb01, CRC(bf17aa05) SHA1(0cbbb2ee49c31e94ed7ea3a25f85e3d33c6b68fb) )
	ROM_LOAD( "d33-05.pal16l8b.ic93.jed", 0x4000, 0xb01, CRC(2df9105e) SHA1(d28fc48588727e4bd3ec2377b383160329c261c3) )
	ROM_LOAD( "d33-06.pal20l8b.ic44.jed", 0x5000, 0xd01, CRC(a1400501) SHA1(0b86d09d3e12668eaaf2c4b5a2d2b676d2e599f7) )
ROM_END

ROM_START( gokidetor2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ic2.rom", 0, 0x20000, CRC(03bf24d1) SHA1(ef63a5be25d77ac20984402cc45137d292a9fa1d) )
	// 8000-FFFF appears to be graphics; 10000-1FFFF is unused

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ic34.rom", 0, 0x10000, CRC(fd06305d) SHA1(7889f0c360650bfd0fe593c522685a978879bfee) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "ic87.rom", 0, 0x40000, CRC(adb9fd16) SHA1(59aae5ac26cae30b961b30e17fd494094aa7aa15) )

	ROM_REGION( 0x8000, "pals", 0 ) // unprotected
	ROM_LOAD( "d33-01.pal16l8a.ic20.jed", 0x0000, 0xb01, CRC(7d614a54) SHA1(e79b6ac99f46ff69ea0565e89d6807667c9e0547) ) // PALs missing for this board, using the one from the parent.
	ROM_LOAD( "d33-02.pal16l8b.ic62.jed", 0x1000, 0xb01, CRC(6653423f) SHA1(ac93acccf649b97a3252aedf398310be90fe5d2d) )
	ROM_LOAD( "d33-03.pal16l8a.ic70.jed", 0x2000, 0xb01, CRC(abfd9a01) SHA1(c76cc16842231c2f688ddd46a17f3a6df92a9580) )
	ROM_LOAD( "d33-04.pal16l8b.ic81.jed", 0x3000, 0xb01, CRC(bf17aa05) SHA1(0cbbb2ee49c31e94ed7ea3a25f85e3d33c6b68fb) )
	ROM_LOAD( "d33-05.pal16l8b.ic93.jed", 0x4000, 0xb01, CRC(2df9105e) SHA1(d28fc48588727e4bd3ec2377b383160329c261c3) )
	ROM_LOAD( "d33-06.pal20l8b.ic44.jed", 0x5000, 0xd01, CRC(a1400501) SHA1(0b86d09d3e12668eaaf2c4b5a2d2b676d2e599f7) )
ROM_END

GAME( 1992, gokidetor,          0, gokidetor, gokidetor, gokidetor_state, empty_init, ROT0, "Taito", "Gokidetor (set 1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1992, gokidetor2, gokidetor, gokidetor, gokidetor, gokidetor_state, empty_init, ROT0, "Taito", "Gokidetor (set 2)", MACHINE_IS_SKELETON_MECHANICAL )
