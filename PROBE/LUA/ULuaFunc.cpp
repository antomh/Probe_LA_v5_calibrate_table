//---------------------------------------------------------------------------

#pragma hdrstop

#include "ULuaFunc.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

bool   stop 	= false;
bool   wait 	= false;
int	   debug	= 0;
int 	state	= STATE_STOP;;

extern Log 			console;
extern Analizator*	analizator;
extern Calibrate*	calibrate;
extern Psu*			psu;
extern Mgqs*		mgqs;
extern USignals*	Signals;

extern u32	access_count;
extern u32	access_error;

//==============================================================================
void	access_event(bool not_error = true){

	access_count++;
	if(!not_error){
		access_error++;
        (*Signals)["Probe.Error"]->Execute("");
	}
	(*Signals)["Probe.Access"]->Execute("");
};

//==============================================================================

void  Wrap(String source, TStrings* receiver, String delimiters){

	enum TMode {lexem,delim};
	TMode mode	=	delim;
	receiver->Clear(); //?????????????

	String Lex;
	for(int i = 1; i <= source.Length(); i++){

		if(source.IsDelimiter(delimiters,i)){
			if(mode == lexem){
				receiver->Add(Lex);
				Lex = "";
			}
			mode = delim;
		}else{
			mode = lexem;
			Lex += source[i];
		}
	}
	if(mode == lexem){
		receiver->Add(Lex);
		Lex = "";
	}
};

//-----------------------------------------
HList	Wrap(String source){

	enum TMode {lexem,delim};
	TMode mode = delim;

	HList	list(true);
	list.Initialize();

	String Lex;
	for(int i = 1;i <= source.Length(); i++){
		if(source.IsDelimiter("/",i)){
			if(mode == lexem){
				list->Add(Lex);
				Lex = "";
			}
			mode = delim;
		}else{
			mode = lexem;
			Lex += source[i];
		}
	}
	if(mode == lexem){
		list->Add(Lex);
		Lex = "";
	}
	return list;
};

//-----------------------------------------
int	Return(lua_State*	lvm){

	stop = true;
	return lua_yield(lvm,0);
}

//-----------------------------------------
int	Pause(lua_State*	lvm){

	return lua_yield(lvm,0);
};

//-----------------------------------------
int	Message(lua_State*	lvm){

	String type;
	String message;
	try{
		int count = lua_gettop(lvm);
		if(count == 1){

			if(lua_isboolean(lvm,1)){

				bool a = lua_toboolean(lvm,1);
				if(a) 	message = "True";
				else	message	= "False";
			}else	message 	= lua_tostring(lvm,1);
			ShowMessage(message);

		}else if(count == 2){

			type 		= lua_tostring(lvm,2);
			message 	= lua_tostring(lvm,1);
			int t = 4;
			if(LowerCase(type) == "warning")		t = 0;
			else  if(LowerCase(type) == "error")	t = 1;
			else  if(LowerCase(type) == "inform")		t = 2;

			MessageDlg(message,TMsgDlgType(t),TMsgDlgButtons()<<mbOK,0);

		}

	}catch(...){;}

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;

};

//-----------------------------------------
int	Console(lua_State*	lvm){

	String type;
	String message;
	try{
		int count = lua_gettop(lvm);
		if(count == 1){

			if(lua_isboolean(lvm,1)){

				bool a = lua_toboolean(lvm,1);
				if(a) 	message = "True";
				else	message	= "False";
			}else	message 	= lua_tostring(lvm,1);
			console<<message<<endl;

		}

	}catch(...){;}

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;

};


//---------------------------------------------------------------------------
int USleep(lua_State*	lvm){

	unsigned t;
	int p = lua_tointeger(lvm,1);
	int a = p/50;
	int b = p - a*50;

	for(int i = 0; i < a; i++){

		Sleep(50);
		Application->ProcessMessages();
		if(stop)	return Return(lvm);

	}
	Sleep(b);
	return 1;
}

//==============================================================================
int 	relay		(lua_State*	lvm){

	String type;
	String message;
	try{
		int count = lua_gettop(lvm);
		if(count == 1){

			if(lua_isboolean(lvm,1) || lua_isnumber(lvm,1)){

				int a = lua_tonumber(lvm,1);
				if(a){
					analizator->set_relay(true);
					console<<"-- ����� +12�"<<endl;
				}else{
					analizator->set_relay(false);
					console<<"-- ����� +27�"<<endl;
				};
			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� 'relay()'"<<endl;
				stop	= true;
			}

		}else{
			console<<"-- Error: ���������� ���������� �� ������������� ��������� ������� 'relay()'"<<endl;
			stop	= true;
		}

	}catch(...){;}

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;

};

//==============================================================================
int 	relay_on		(lua_State*	lvm){

	String type;
	String message;
	try{
		bool res   = analizator->set_relay(true);
		access_event(res);
		if(res){
			console<<"-- ����� +12�"<<endl;
		}else{
			console<<"������ ��������� � ����������"<<endl;
		}

	}catch(...){;}

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;

};

//==============================================================================
int 	relay_off		(lua_State*	lvm){

	String type;
	String message;
	try{
		bool res   = analizator->set_relay(false);
		access_event(res);
		if(res){
			console<<"-- ����� +27�"<<endl;
		}else{
			console<<"������ ��������� � ����������"<<endl;
		}
	}catch(...){;}

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;

};

//==============================================================================
int 	low(lua_State*	lvm){

	String type;
	String message;
	try{
		int count = lua_gettop(lvm);
		if(count == 1){

			if(lua_isnumber(lvm,1)){

				double a 	= lua_tonumber(lvm,1);
				bool res	= analizator->set_level(0,a*1000.0);	//mV
				access_event(res);
				if(res){
					console<<"-- ������ ������� ��������������: " + FloatToStr(analizator->Level[0]/1000.0) + "�"<<endl;
				}else{
					console<<"������ ��������� � ����������"<<endl;
				}
			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� low()"<<endl;
				stop	= true;
			}

		}else{
			console<<"Error: ���������� ���������� �� ������������� ��������� ������� low()"<<endl;
			stop	= true;
		}

	}catch(...){;}

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};

//==============================================================================
int 	low_code(lua_State*	lvm){

	String type;
	String message;
	try{
		int count = lua_gettop(lvm);
		if(count == 1){

			if(lua_isnumber(lvm,1)){

				i32 a 	= lua_tointeger(lvm,1);
				bool res	= analizator->set_code(0, a);	//mV
				access_event(res);
				if(res){
//					console<<"-- ������ ������� ��������������: " + IntToStr(a)<<endl;
				}else{
//					console<<"������ ��������� � ����������"<<endl;
				}
			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� low_code()"<<endl;
				stop	= true;
			}

		}else{
			console<<"Error: ���������� ���������� �� ������������� ��������� ������� low_code()"<<endl;
			stop	= true;
		}

	}catch(...){;}

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};

//==============================================================================
int 	hi(lua_State*	lvm){

	String type;
	String message;
	try{
		int count = lua_gettop(lvm);
		if(count == 1){

			if(lua_isnumber(lvm,1)){

				double a 	= lua_tonumber(lvm,1);
				bool res	= analizator->set_level(1,a*1000.0);	//mV
				access_event(res);
				if(res){
					console<<"-- ������� ������� ��������������: " + FloatToStr(analizator->Level[1]/1000.0) + "�"<<endl;
				}else{
					console<<"������ ��������� � ����������"<<endl;
				}
			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� hi()"<<endl;
				stop	= true;
			}

		}else{
			console<<"Error: ���������� ���������� �� ������������� ��������� ������� hi()"<<endl;
			stop	= true;
		}

	}catch(...){;}

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};

//==============================================================================
int 	hi_code(lua_State*	lvm){

	String type;
	String message;
	try{
		int count = lua_gettop(lvm);
		if(count == 1){

			if(lua_isnumber(lvm,1)){

				i32 a 	= lua_tonumber(lvm,1);
				bool res	= analizator->set_code(1,a);	//mV
				access_event(res);
				if(res){
//					console<<"-- ������� ������� ��������������: " + IntToStr(a)<<endl;
				}else{
//					console<<"������ ��������� � ����������"<<endl;
				}
			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� hi_code()"<<endl;
				stop	= true;
			}

		}else{
			console<<"Error: ���������� ���������� �� ������������� ��������� ������� hi_code()"<<endl;
			stop	= true;
		}

	}catch(...){;}

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};

//==============================================================================
int 	set_code(lua_State*	lvm){

	String type;
	String message;
	try{
		int count = lua_gettop(lvm);
		if(count == 2){

			if(lua_isnumber(lvm,1) && lua_isnumber(lvm,2)){

				i32 num 	= lua_tonumber(lvm,1);
				i32 code	= lua_tonumber(lvm,2);
				bool res	= analizator->set_code(num,code);	//mV
				access_event(res);
				if(res){
//					console<<"-- ������� ������� ��������������: " + IntToStr(a)<<endl;
				}else{
//					console<<"������ ��������� � ����������"<<endl;
				}
			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� set_code()"<<endl;
				stop	= true;
			}

		}else{
			console<<"Error: ���������� ���������� �� ������������� ��������� ������� set_code()"<<endl;
			stop	= true;
		}

	}catch(...){;}

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};



//==============================================================================
int 	get_event(lua_State*	lvm){

	try{
		int count = lua_gettop(lvm);

		if(count == 1){

			if(lua_isnumber(lvm,1)){
				int	num		= lua_tointeger(lvm,1);
				u16 event	= 0;
				bool res	= analizator->getEvent(num, event);
				access_event(res);

				if(res){
					lua_pushnumber(lvm,event);
				}else{
//					console<<"������ ��������� � ����������"<<endl;
				}

			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� get_event()"<<endl;
				stop	= true;
			}
		}else{
			console<<"Error: ���������� ���������� �� ������������� ��������� ������� get_event()"<<endl;
			stop	= true;
		}

	}catch(...){};

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};


//==============================================================================
int 	set_voltage(lua_State*	lvm){

	String type;
	String message;
	try{
		int count = lua_gettop(lvm);
		if(count == 1){

			if(lua_isnumber(lvm,1)){

				double volt 	= lua_tonumber(lvm,1);
				bool res		= true;
					res = res && psu->set_voltage(1, volt);
					res = res && psu->set_voltage(2, volt);
					res = res && psu->set_voltage(3, volt);
					res = res && psu->set_voltage(4, volt);
					access_event(res);
				if(res){
					console<<"-- �� ������� 1..4 ����������� ���������� : " + FloatToStr(int(100*volt)/1000.0) + "�"<<endl;
				}else{
					console<<"������ ��������� � ��"<<endl;
				}
			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� set_voltage()"<<endl;
				stop	= true;
			}

		}else if(count == 2){

			if(lua_isnumber(lvm,1) && lua_isnumber(lvm,2)){

				i32	   index	= lua_tonumber(lvm,1);
				double volt 	= lua_tonumber(lvm,2);
				bool   res 		= psu->set_voltage(index, volt);

					access_event(res);
				if(res){
					console<<"-- �� ������" + IntToStr(index) + " ����������� ���������� : " + FloatToStr(volt) + "�"<<endl;
				}else{
					console<<"������ ��������� � ��"<<endl;
				}
			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� set_voltage()"<<endl;
				stop	= true;
			}

		}else{
			console<<"Error: ���������� ���������� �� ������������� ��������� ������� hi()"<<endl;
			stop	= true;
		}

	}catch(...){;}

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};

//-------------------------------------
int 	set_current(lua_State*	lvm){

	String type;
	String message;
	try{
		int count = lua_gettop(lvm);
		if(count == 1){

			if(lua_isnumber(lvm,1)){

				double current	= lua_tonumber(lvm,1);
				bool res		= true;
					res = res && psu->set_current(1, current);
					res = res && psu->set_current(2, current);
					res = res && psu->set_current(3, current);
					res = res && psu->set_current(4, current);
					access_event(res);
				if(res){
					console<<"-- �� ������� 1..4 ���������� ��� : " + FloatToStr(int(100*current)/1000.0) + "�"<<endl;
				}else{
					console<<"������ ��������� � ��"<<endl;
				}
			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� set_voltage()"<<endl;
				stop	= true;
			}

		}else if(count == 2){

			if(lua_isnumber(lvm,1) && lua_isnumber(lvm,2)){

				i32	   index	= lua_tonumber(lvm,1);
				double current 	= lua_tonumber(lvm,2);
				bool   res 		= psu->set_current(index, current);

					access_event(res);
				if(res){
					console<<"-- �� ������" + IntToStr(index) + " ����������� ���������� : " + FloatToStr(current) + "�"<<endl;
				}else{
					console<<"������ ��������� � ��"<<endl;
				}
			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� set_voltage()"<<endl;
				stop	= true;
			}

		}else{
			console<<"Error: ���������� ���������� �� ������������� ��������� ������� hi()"<<endl;
			stop	= true;
		}

	}catch(...){;}

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};

//-------------------------------------
int 	get_voltage(lua_State*	lvm){

	try{
		int count = lua_gettop(lvm);

		if(count == 1){

			if(lua_isnumber(lvm,1)){
				int	num		= lua_tointeger(lvm,1);
				double U	= 0;
				bool res	= psu->get_voltage(num, U);
				access_event(res);

				if(res){
					lua_pushnumber(lvm,U);
				}else{
					console<<"������ ��������� � ��"<<endl;
				}

			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� get_voltage()"<<endl;
				stop	= true;
			}
		}else{
			console<<"Error: ���������� ���������� �� ������������� ��������� ������� get_voltage()"<<endl;
			stop	= true;
		}

	}catch(...){};

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};

//-------------------------------------
int 	get_current(lua_State*	lvm){

	try{
		int count = lua_gettop(lvm);

		if(count == 1){

			if(lua_isnumber(lvm,1)){
				int	num		= lua_tointeger(lvm,1);
				double I	= 0;
				bool res	= psu->get_current(num, I);
				access_event(res);

				if(res){
					lua_pushnumber(lvm,I);
				}else{
					console<<"������ ��������� � ��"<<endl;
				}

			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� get_current()"<<endl;
				stop	= true;
			}
		}else{
			console<<"Error: ���������� ���������� �� ������������� ��������� ������� get_current()"<<endl;
			stop	= true;
		}

	}catch(...){};

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};

//-------------------------------------
int 	enable(lua_State*	lvm){

	try{
		int count = lua_gettop(lvm);

		if(count == 0){

			bool res = true;
			res = res && psu->enable(1);
			res = res && psu->enable(2);
			res = res && psu->enable(3);
			res = res && psu->enable(4);

			if(!res){
				console<<"������ ��������� � ��"<<endl;
			}

		}else if(count == 1){

			if(lua_isnumber(lvm,1)){
				int	num		= lua_tointeger(lvm,1);
				double I	= 0;
				bool res	= psu->enable(num);
				access_event(res);

				if(res){
					console<<"������ ��������� � ��"<<endl;
				}

			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� get_current()"<<endl;
				stop	= true;
			}
		}else{
			console<<"Error: ���������� ���������� �� ������������� ��������� ������� get_current()"<<endl;
			stop	= true;
		}

	}catch(...){};

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};

//-------------------------------------
int 	disable(lua_State*	lvm){

	try{
		int count = lua_gettop(lvm);

		if(count == 0){

			bool res = true;
			res = res && psu->disable(1);
			res = res && psu->disable(2);
			res = res && psu->disable(3);
			res = res && psu->disable(4);

			if(!res){
				console<<"������ ��������� � ��"<<endl;
			}

		}else if(count == 1){

			if(lua_isnumber(lvm,1)){
				int	num		= lua_tointeger(lvm,1);
				double I	= 0;
				bool res	= psu->disable(num);
				access_event(res);

				if(res){
					console<<"������ ��������� � ��"<<endl;
				}

			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� get_current()"<<endl;
				stop	= true;
			}
		}else{
			console<<"Error: ���������� ���������� �� ������������� ��������� ������� get_current()"<<endl;
			stop	= true;
		}

	}catch(...){};

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};

//==============================================================================
int 	create(lua_State*	lvm){

	String type;
	String message;
	try{
		calibrate->table_create();
		console<<"������� ����������� �������������������"<<endl;

	}catch(...){;}

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};

//--------------------------------------------------
int 	close(lua_State*	lvm){

	String type;
	String message;
	try{
		calibrate->table_close();
		console<<"������� ����������� ������������ ��� ����������"<<endl;

	}catch(...){;}

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};

//--------------------------------------------------
int 	add_12v(lua_State*	lvm){

	String type;
	String message;
	try{
		int count = lua_gettop(lvm);
		if(count == 3){

			if(lua_isnumber(lvm,1) && lua_isnumber(lvm,2) && lua_isnumber(lvm,3)){

				i32	   index	= lua_tointeger(lvm,1);
				double volt 	= lua_tonumber(lvm,2);
				i32 code 		= lua_tointeger(lvm,3);

				calibrate->table_add_12v(index, volt*1000, code);

			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� add_12v()"<<endl;
				stop	= true;
			}

		}else{
			console<<"Error: ���������� ���������� �� ������������� ��������� ������� add_12v()"<<endl;
			stop	= true;
		}

	}catch(...){;}

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};

//--------------------------------------------------
int 	add_27v(lua_State*	lvm){

	String type;
	String message;
	try{
		int count = lua_gettop(lvm);
		if(count == 3){

			if(lua_isnumber(lvm,1) && lua_isnumber(lvm,2) && lua_isnumber(lvm,3)){

				i32	   index	= lua_tointeger(lvm,1);
				double volt 	= lua_tonumber(lvm,2);
				i32 code 		= lua_tointeger(lvm,3);

				calibrate->table_add_27v(index, volt*1000, code);

			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� add_27v()"<<endl;
				stop	= true;
			}

		}else{
			console<<"Error: ���������� ���������� �� ������������� ��������� ������� add_27v()"<<endl;
			stop	= true;
		}

	}catch(...){;}

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};

//--------------------------------------------------
int 	to_code_12v(lua_State*	lvm){

	try{
		int count = lua_gettop(lvm);

		if(count == 1){

			if(lua_isnumber(lvm,1)){
				double	volt	= 1000*lua_tonumber(lvm,1);
				int code		= calibrate->volt_to_code_12v(volt);

				lua_pushnumber(lvm,code);

			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� to_code_12v()"<<endl;
				stop	= true;
			}
		}else{
			console<<"Error: ���������� ���������� �� ������������� ��������� ������� to_code_12v()"<<endl;
			stop	= true;
		}

	}catch(...){};

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};

//--------------------------------------------------
int		to_code_27v(lua_State*	lvm){

	try{
		int count = lua_gettop(lvm);

		if(count == 1){

			if(lua_isnumber(lvm,1)){
				double	volt	= 1000*lua_tonumber(lvm,1);
				int code		= calibrate->volt_to_code_27v(volt);

				lua_pushnumber(lvm,code);

			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� to_code_27v()"<<endl;
				stop	= true;
			}
		}else{
			console<<"Error: ���������� ���������� �� ������������� ��������� ������� to_code_27v()"<<endl;
			stop	= true;
		}

	}catch(...){};

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};

//--------------------------------------------------
int		save(lua_State*	lvm){


	try{
		int count = lua_gettop(lvm);

		if(count == 0){
			calibrate->table_close();
			calibrate->save_to_file(DEFAULT_CALIBRATE_TABLE);
		}else if(count == 1){

			if(lua_isstring(lvm,1)){
				calibrate->table_close();
				String path	= lua_tostring(lvm,1);
				calibrate->save_to_file(path);

			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� save()"<<endl;
				stop	= true;
			}
		}else{
			console<<"Error: ���������� ���������� �� ������������� ��������� ������� save()"<<endl;
			stop	= true;
		}

	}catch(...){};

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};

//==============================================================================
int 	set_amplitude(lua_State*	lvm){

	try{
		int count = lua_gettop(lvm);
		if(count == 2){

			if(lua_isnumber(lvm,1) && lua_isnumber(lvm,2)){

				i32	   index		= lua_tonumber(lvm,1);
				double amplitude 	= lua_tonumber(lvm,2);
				bool   res 			= mgqs->set_amplitude(index, amplitude);

					access_event(res);
				if(res){
					console<<"-- �� ������" + IntToStr(index) + " ����������� ��������� : " + FloatToStr(amplitude) + "�"<<endl;
				}else{
					console<<"������ ��������� � ����"<<endl;
				}
			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� set_amplitude()"<<endl;
				stop	= true;
			}

		}else{
			console<<"Error: ���������� ���������� �� ������������� ��������� ������� set_amplitude()"<<endl;
			stop	= true;
		}

	}catch(...){;}

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};

//==============================================================================
int 	set_offset(lua_State*	lvm){

	try{
		int count = lua_gettop(lvm);
		if(count == 2){

			if(lua_isnumber(lvm,1) && lua_isnumber(lvm,2)){

				i32	   index		= lua_tonumber(lvm,1);
				double offset 		= lua_tonumber(lvm,2);
				bool   res 			= mgqs->set_offset(index, offset);

					access_event(res);
				if(res){
					console<<"-- �� ������" + IntToStr(index) + " ����������� ��������: " + FloatToStr(offset) + "�"<<endl;
				}else{
					console<<"������ ��������� � ����"<<endl;
				}
			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� set_offset()"<<endl;
				stop	= true;
			}

		}else{
			console<<"Error: ���������� ���������� �� ������������� ��������� ������� set_offset()"<<endl;
			stop	= true;
		}

	}catch(...){;}

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;

};

//==============================================================================
int 	mgqs_enable(lua_State*	lvm){

	try{
		int count = lua_gettop(lvm);

		if(count == 0){

			bool res = true;
			res = res && mgqs->enable(1);
			res = res && mgqs->enable(2);

			if(!res){
				console<<"������ ��������� � ����"<<endl;
			}

		}else if(count == 1){

			if(lua_isnumber(lvm,1)){
				int	num		= lua_tointeger(lvm,1);
				bool res	= mgqs->enable(num + 1);
				access_event(res);

				if(res){
					console<<"������ ��������� � ����"<<endl;
				}

			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� mgqs_enable()"<<endl;
				stop	= true;
			}
		}else{
			console<<"Error: ���������� ���������� �� ������������� ��������� ������� mgqs_disable()"<<endl;
			stop	= true;
		}

	}catch(...){};

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};

//==============================================================================
int 	mgqs_disable(lua_State*	lvm){

 	try{
		int count = lua_gettop(lvm);

		if(count == 0){

			bool res = true;
			res = res && mgqs->disable(1);
			res = res && mgqs->disable(2);

			if(!res){
				console<<"������ ��������� � ����"<<endl;
			}

		}else if(count == 1){

			if(lua_isnumber(lvm,1)){
				int	num		= lua_tointeger(lvm,1);
				bool res	= mgqs->disable(num + 1);
				access_event(res);

				if(res){
					console<<"������ ��������� � ����"<<endl;
				}

			}else{
				console<<"-- Error: ���� ���������� �� ������������� ��������� ������� mgqs_disable()"<<endl;
				stop	= true;
			}
		}else{
			console<<"Error: ���������� ���������� �� ������������� ��������� ������� mgqs_disable()"<<endl;
			stop	= true;
		}

	}catch(...){};

	Application->ProcessMessages();
	if(stop)	return Return(lvm);
	return 1;
};
