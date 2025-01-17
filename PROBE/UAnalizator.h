//---------------------------------------------------------------------------

#ifndef UAnalizatorH
#define UAnalizatorH
//#include "UPoints.h"
#include "Probe.h"
#include "Definition.h"
#include "Templates.h"
#include "ULog.h"
#include "UData.h"
#include "USignals.h"
//---------------------------------------------------------------------------
const std::string	DEFAULT_ID	= "prb_v0.3";
const std::string	DEFAULT_NEW_ID	= "SN";

const i32	V12_MIN	= -12000;
const i32	V12_MAX	= 12000;

const i32	V27_MIN	= -27000;
const i32	V27_MAX	= 27000;

const i32	CODE_MIN	= 0;
const i32	CODE_MAX	= 4096;

const String DEFAULT_CALIBRATE_TABLE	= "logic_calibration_table.txt";
const String DEFAULT_PORT				= "logic_port.txt";

//==============================================================================
class Analizator{

public:
	Analizator();
	~Analizator();

	__property String	Address				= {read = _address, 	write = _address};
	__property u8		Ols[u8 index]		= {read = get_ols, 		write = set_ols};
	__property u8		Contact[u8 index]	= {read = get_contact, 	write = set_contact};
	__property i32		Level[u8 index]		= {read = get_level, 	write = set_level};
	__property bool		Relay				= {read = get_relay, 	write = set_relay};
	__property i32		TargetId 			= {read = _target_id, 	write = _target_id};

	void	__fastcall connect(std::string address, i32 baudrate = 9600);
	void	__fastcall disconnect();
	bool	__fastcall is_connected();
	bool 	__fastcall getDeviceID(String &ID);
	bool	__fastcall getEvent(u8 ch, u16& event);

	void	__fastcall comparator_ON();
	void	__fastcall comparator_OFF();
	bool	__fastcall used_comparator();

	void	__fastcall	accumulation_ON(){_accumulation	= true;};
	void	__fastcall	accumulation_OFF(){_accumulation	= false;};
	bool	__fastcall 	accumulation(){return _accumulation;};

	void	__fastcall state(bool& relay, u16& dac_a, u16& dac_b);

	void	__fastcall complete();
	u8		__fastcall calculate(bool low, bool high);

	//-------------
	void	__fastcall initialize_maps();
	void	__fastcall inner_map_initialize();
	void	__fastcall save_map_to_file(String filename);
	void	__fastcall load_map_from_file(String filename);
	HList	__fastcall wrap(String rules);

	i32		__fastcall volt_to_code(u8 ch, i32 volt, bool relay  = false);
	i32		__fastcall code_to_volt(u8 ch, bool relay, u32 code);

	void	__fastcall	save_port_info();
	void	__fastcall	load_port_info();
	//-------------

	HList			get_COM_ports();
	bool setTableCount(u8 mode, i16 min, i16 max, u16 count);
	bool setTablePacket(u8 mode, u16 offset, u16 count, u16 code[], u16 pack_size);

private:

static	u8				_ols[2];
static	u32				_contact[2];
static	i32				_level[2];

		i32				_target_id;

static	String			_address;
static	bool			_comparator_on;
static	bool			_relay;
static	bool			_accumulation;

static	Probe 				_probe;
static	bool				_lock;

static	std::map<i32, pair<i32, i32> >  low;
static  std::map<i32, pair<i32, i32> >  hi;

	u8		__fastcall 	get_ols(u8 index);
	u8		__fastcall 	get_contact(u8 index);
	i32		__fastcall  get_level(u8 index);
	bool	__fastcall  get_relay();

	void	__fastcall	set_ols(u8 index, u8 ols);
	void	__fastcall	set_contact(u8 index, u8 contact);
public:
	bool	__fastcall	set_level(u8 index, i32 level);
	bool	__fastcall	set_code(u8 index, i32 code);
	bool	__fastcall	set_relay(bool enable);
private:
	std::string		get_first(HList com_ports);

	void	__fastcall	message(String event,String data = EmptyStr);
	void	__fastcall	lock();
	void	__fastcall 	unlock();

};

u8 		Analizator::_ols[]		= {1,1};
u32 	Analizator::_contact[]	= {31,32};
i32 	Analizator::_level[]	= {400, 2400};

String	Analizator::_address		= EmptyStr;
bool    Analizator::_comparator_on	= true;
bool    Analizator::_relay			= true;
bool	Analizator::_accumulation	= false;

Probe 	Analizator::_probe;

bool	Analizator::_lock	= false;

std::map<i32, pair<i32, i32> >  Analizator::low;
std::map<i32, pair<i32, i32> >  Analizator::hi;


#endif
