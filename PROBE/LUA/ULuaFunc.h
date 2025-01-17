//---------------------------------------------------------------------------

#ifndef ULuaFuncH
#define ULuaFuncH
#include "lua.hpp"
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Dialogs.hpp>
#include "Templates.h"
#include "ULog.h"
#include "UAnalizator.h"
#include "UPsu.h"
#include "UCalibrate.h"
#include "UMgqs.h"

//---------------------------------------------------------------------------
const int 		CHECK_GOOD		= 1;
const int   	CHECK_BAD		= 0;
const int   	CHECK_UNKNOWN	= -1;

const int		STATE_STOP		= 0;
const int		STATE_PROCEED	= 1;
const int		STATE_PAUSED 	= 2;
const int		STATE_WAIT 		= 3;

const String	WAIT	=  	"WAIT";
const String	STOP	= 	"STOP";
const String	PROCEED	=	"PROCEED";
const String	PAUSED	=	"PAUSED";


//==============================================================================

//---- ��������������� ������� -----------
void  	Wrap(String source, TStrings* receiver, String delimiters);
HList	Wrap(String source);

//---- ��������� ������� -------------------------
int		Return		(lua_State*	lvm);
int		Pause		(lua_State*	lvm);
int		Message		(lua_State*	lvm);
int 	USleep		(lua_State*	lvm);
int 	Console		(lua_State*	lvm);

//-- la --------------------
int 	relay_on		(lua_State*	lvm);
int 	relay_off		(lua_State*	lvm);

int 	low			(lua_State*	lvm);
int 	hi			(lua_State*	lvm);
int 	low_code	(lua_State*	lvm);
int 	hi_code		(lua_State*	lvm);
int 	set_code   	(lua_State*	lvm);

int 	get_event(lua_State*	lvm);

//-- psu -----------------
int 	set_voltage(lua_State*	lvm);
int 	set_current(lua_State*	lvm);

int 	get_voltage(lua_State*	lvm);
int 	get_current(lua_State*	lvm);

int 	enable(lua_State*	lvm);
int 	disable(lua_State*	lvm);

//--- calibrate -----------------
int 	create(lua_State*	lvm);
int 	close(lua_State*	lvm);
int 	add_12v(lua_State*	lvm);
int 	add_27v(lua_State*	lvm);
int 	to_code_12v(lua_State*	lvm);
int		to_code_27v(lua_State*	lvm);
int		to_code_27v(lua_State*	lvm);
int		save(lua_State*	lvm);

//-----mgqs-----------------
int 	set_amplitude(lua_State*	lvm);
int 	set_offset(lua_State*	lvm);
int 	mgqs_enable(lua_State*	lvm);
int 	mgqs_disable(lua_State*	lvm);



#endif
