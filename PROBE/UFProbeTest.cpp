//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "UFProbeTest.h"
#include "Registry.hpp"
#include "ULua.h"
#include "ULog.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm3 *Form3;

extern USignals*	Signals;
extern Log			console;
extern Analizator* 	analizator;
extern Calibrate*	calibrate;
extern Psu*			psu;
extern Mgqs*	 	mgqs;

extern u32	access_count;
extern u32	access_error;

extern bool stop;

String TAG_START	= "--functions";
String TAG_END		= "--end_functions";

String VERSION = "1.1.0.7";
//============================================================================
String minimize_text(const String& text, TCanvas* canvas, int width){
// Log("MinimizeText");

	String ellipsis	= "�";

	int text_width	= canvas->TextWidth(text);
	if(text_width < width)	return text;

	String buf = text + ellipsis;
	do{
		buf= buf.SubString(1, buf.Length() - 2) + ellipsis;
		if(buf.Length() == 1)	break;
	}while(canvas->TextWidth(buf) > width);
	return buf;
};

/*
//===========================================================================
class Console{

public:
	Console(TListBox* owner = NULL){_owner	= owner;};
	Console&	operator<<(String info){if(_owner) _owner->Items->Add(info); _owner->ItemIndex	=_owner->Count -1;  return *this;};
	void		clear(){if(_owner)	_owner->Items->Clear();};
	void		set_owner(TListBox* owner){_owner	= owner;};
private:
	TListBox*	_owner;
};

Console console;
*/

//---------------------------------------------------------------------------
__fastcall TForm3::TForm3(TComponent* Owner): TForm(Owner), _slots(Signals){

	Caption = "ProbeTest " + VERSION;

	Signals->Create("Log.Change");
	Signals->Create("Probe.Perform");
	Signals->Create("Probe.Config");
	Signals->Create("Probe.Access");
	Signals->Create("Probe.Error");

	_slots.Connect("Log.Change")->OnHandler	= OnLogChange;
	_slots.Connect("Probe.Access")->OnHandler	= OnProbeAccess;
	_slots.Connect("Probe.Error")->OnHandler	= OnProbeError;
	console.init();

	set_data();
}

//-----------------------------------
void	__fastcall TForm3::OnLogChange(String message){

	try{
		ListBox2->Items->Insert(0, message);
	}catch(...){

	}
};

//-----------------------------------
void	__fastcall TForm3::OnProbeAccess(String message){

	try{
		StatusBar1->Panels->Items[0]->Text	= "���������� ���������: " + IntToStr((int)access_count);
		StatusBar1->Panels->Items[1]->Text	= "������: " + IntToStr((int)access_error);
	}catch(...){

	}
};

//----------------------------------
void	__fastcall TForm3::OnProbeError(String data){

	try{
		analizator->disconnect();
		if (!analizator->is_connected()) {
			if(Box1->ItemIndex != -1){
				analizator->connect(AnsiString(Box1->Items->Strings[Box1->ItemIndex]).c_str(), CBR_9600);
			}
		}
		if(analizator->is_connected())	Button3->Caption	= "�����������";
		else 							Button3->Caption	= "������������";
	}catch(...){
	}
};

//---------------------------------------------------------------------------
HList			TForm3::get_COM_ports(){

	String KeyName	= "\\Hardware\\DeviceMap\\SerialComm";

	HList keys(true);
	HList values(true);
	HPtr<TRegistry>	registry(true);

	try{
		registry->RootKey	= HKEY_LOCAL_MACHINE;
		registry->OpenKeyReadOnly(KeyName);
		registry->GetValueNames(keys);

		for (int i = 0; i < keys->Count; i++) {
			values->Add(registry->ReadString(keys->Strings[i]));
		}

	}catch(...){
		console<<"�� ������� �������� ������ ��� ������."<<endl;
	}
	return values;
};

//-----------------------------------
void	__fastcall TForm3::show_ports(){

	HList list	= get_COM_ports();
	Box1->Items->Clear();
	Box1->Items->Assign(list);
};

//------------------------------------
void __fastcall TForm3::FormCreate(TObject *Sender)
{
	show_ports();
	psu->open_dll("N6702C");
	mgqs->open_dll("triangle_mgqs.dll");
	if(mgqs->connect(0,1,1))	console<<"��������� ���������"<<endl;
	mgqs->enable(1);
	if(FileExists("Default.scr"))  load("Default.scr");
//	console<<"Application is opened"<<endl;
}
//---------------------------------------------------------------------------

void __fastcall TForm3::Button3Click(TObject *Sender)
{
	if (!analizator->is_connected())		connect();
	else								disconnect();

}
//---------------------------------------------------------------------------
String	__fastcall	TForm3::getDeviceId(){

	String ID;

	if (!analizator->getDeviceID(ID))    console<<"������: ������ id ��������"<<endl;
	else {
		String id_info	= ID;
		if(id_info	== DEFAULT_ID.c_str())	console<<"ID ����������: " + id_info<<endl;
		else						console<<"������: ������������ id ����������"<<endl;

		return id_info;
	}
	return EmptyStr;
};

//------------------------------------
void	__fastcall 	TForm3::connect(){

	if (!analizator->is_connected()) {
		DWORD baudRate	= CBR_9600;
		if(Box1->ItemIndex == -1){
			console<<"�� ������ COM ����"<<endl;
			return;
		}
		analizator->connect(AnsiString(Box1->Items->Strings[Box1->ItemIndex]).c_str(), baudRate);
		if(!analizator->is_connected()) {
			console<<"������ �����������: " + String(_probe.getLastError().c_str())<<endl;
			return;
		}
		console<<"���������� ����������. ";
		getDeviceId();
		Button3->Caption	= "�����������";

	}
};

//------------------------------------
void	__fastcall	TForm3::disconnect(){

	analizator->disconnect();
	Button3->Caption	= "������������";
	console<<"���������� ���������"<<endl;
	console<<"--------------------"<<endl;
};

//------------------------------------
void	__fastcall	TForm3::connect_psu(){

	if(!psu->is_connected()){
		String address	= Edit3->Text;
		i32 port		= Edit4->Text.ToIntDef(5024);
		psu->connect(address, port);
		if(!psu->is_connected()) {
			console<<"������ ����������� ��."<<endl;
			return;
		}
		console<<"�� ���������."<<endl;
		Button5->Caption	= "�����������";
	};
};

//------------------------------------
void	__fastcall	TForm3::disconnect_psu(){

	psu->disconnect();
	Button5->Caption	= "������������";
	console<<"�� ��������"<<endl;
	console<<"--------------------"<<endl;
};

//----------------------------------------------
String	__fastcall TForm3::edit_control(String text){

	bool first	= true;
	String S	= "";
	if(text == "") return "0";
	if(isdigit(text[1])|text[1] == '+')	S += text[1];

	for(int i = 2;i <= text.Length(); i++){
		if(isdigit(text[i]))	S += text[i];
		else if(first){
				S 		+= '.';
				first	=  false;
			}
	}
	return S;
};

//----------------------------------------------
void	__fastcall TForm3::get_data(){

	analizator->Level[0]	= _low;
	analizator->Level[1]	= _hi;
};

//----------------------------------------------
void	__fastcall TForm3::set_data(){

	CheckBox1->Checked	= !analizator->Relay;

	_low	= analizator->Level[0];
	_hi		= analizator->Level[1];

	Edit1->Text	= FloatToStr(int(_low/100)/10.0);
	Edit2->Text = FloatToStr(int(_hi/100)/10.0);
};

//----------------------------------------------
void	__fastcall TForm3::accept(){

	get_data();
	analizator->Level[0]	= _low;
	analizator->Level[1]	= _hi;
};

//----------------------------------------------
void	__fastcall TForm3::set_low(i32 low){

		if(low < 0){
			_low 	= 0;
			return;
		}
		if(low > _hi){
			_low	= _hi;
			return;
		}

		if(low > 9000 && analizator->Relay)			_low 	= 9000;
		else if(low < 9000 && !analizator->Relay)	_low 	= 9000;
		else if(low > 27000)						_low	= 27000;
		else 										_low	= low;
};

//----------------------------------------------
void	__fastcall TForm3::set_hi(i32 hi){

		if(hi < 0){
			_hi	= 0;
			return;
		}
		if(hi < _low){
			_hi	= _low;
			return;
		}

		if(hi > 9000 && analizator->Relay)			_hi 	= 9000;
		else if(hi < 9000 && !analizator->Relay)	_hi 	= 9000;
		else if(hi > 27000)							_hi		= 27000;
		else 										_hi		= hi;
};

//----------------------------------------------
void	__fastcall TForm3::scroll(i32 offset){

	TEdit*	edit = Edit1->Focused()? Edit1: Edit2->Focused()? Edit2: NULL;
	if(!edit)	return;

	edit->OnChange	= NULL;
	(edit->Tag == 0)?	set_low(_low + offset): set_hi(_hi + offset);
	Edit1->Text	= FloatToStr(int(_low/100)/10.0);
	Edit2->Text	= FloatToStr(int(_hi/100)/10.0);
	edit->OnChange	= Edit1Change;
};

//----------------------------------------
void	__fastcall TForm3::send_count(){


};

//----------------------------------------
void	__fastcall TForm3::send_data(){


};

//----------------------------------------
vector<pair<i32, i32> >	__fastcall TForm3::load_vect(u8 mode, bool cut){

	vector<pair<i32, i32> >	_vect = calibrate->get_vector(ComboBox1->ItemIndex);
	if(cut){
		_vect.erase(_vect.begin());
		_vect.erase(_vect.begin() + _vect.size() - 1);
	}

	return _vect;
};

//----------------------------------------
void	__fastcall TForm3::show_vect(vector<pair<i32, i32> > vect){

	Memo_out->Clear();
	for(int i = 0; i < vect.size(); i++){

		Memo_out->Lines->Add(IntToStr(vect[i].second));
	}
	if(vect.size())	Edit5->Text = "0";
	else            Edit5->Text = "-1";
	if(vect.size())	Edit6->Text = IntToStr(int(vect.size()));
	else            Edit6->Text = "0";
}

//----------------------------------------
void __fastcall TForm3::ToolButton1Click(TObject *Sender)
{
	execute();
}
//---------------------------------------------------------------------------
void	__fastcall	TForm3::execute(){

	access_count	= 0;
	access_error	= 0;
	String str	= Memo2->Lines->Text + Memo1->Lines->Text;

	String script	= "function Main()" + str + "end";

	ULua lua;

	clock_t	T0		= clock();
	console<<(TimeToStr(Now()) + " ������ �������")<<endl;
	lua.Execute(script);

	clock_t	T1		= clock();
	String run_time	= FloatToStr(1.0*(T1 - T0)/CLK_TCK);
	console<<(TimeToStr(Now()) + " ������ ��������. ����� ����������: " + run_time + " c")<<endl;

};


//----------------------------------------------
void	__fastcall	TForm3::save(String filename){

	if(filename == EmptyStr)	return;
	String ext	= ExtractFileExt(filename);
	if(ext == EmptyStr)	filename += ".scr";

	HList	list(true);
	if(Memo2->Lines->Text != EmptyStr){
		list->Add(TAG_START);
		list->AddStrings(Memo2->Lines);
		list->Add(TAG_END);
	}
	list->AddStrings(Memo1->Lines);
	list->SaveToFile(filename);

};

//----------------------------------------------
void	__fastcall	TForm3::load(String filename){

	Memo1->Clear();
	Memo2->Clear();
	HList list(true);
	list->LoadFromFile(filename);
	if(!list->Count || list->Strings[0] != TAG_START){
		Memo1->Lines->Text	= list->Text;
	}else{
		TStrings*	strings = Memo2->Lines;
		for(int  i = 1; i < list->Count; i++){
			if(list->Strings[i] == TAG_END){
				strings = Memo1->Lines;
				continue;
			};
			strings->Add(list->Strings[i]);
		}
    }
};

//----------------------------------------------
void __fastcall TForm3::N1Click(TObject *Sender)
{
	Memo1->CutToClipboard();
}
//---------------------------------------------------------------------------

void __fastcall TForm3::N2Click(TObject *Sender)
{
	Memo1->CopyToClipboard();
}
//---------------------------------------------------------------------------

void __fastcall TForm3::N3Click(TObject *Sender)
{
	Memo1->PasteFromClipboard();
}
//---------------------------------------------------------------------------

void __fastcall TForm3::N4Click(TObject *Sender)
{
	Memo1->SelectAll();
}
//---------------------------------------------------------------------------

void __fastcall TForm3::ToolButton2Click(TObject *Sender)
{
	if(SaveDlg->Execute()){
		if(SaveDlg->FileName !=""){
			save(SaveDlg->FileName);
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm3::ToolButton3Click(TObject *Sender)
{
	OpenDlg->Execute();

	try{
		if(FileExists(OpenDlg->FileName))
		load(OpenDlg->FileName);
	}catch(...){ return;};
}
//---------------------------------------------------------------------------

void __fastcall TForm3::FormMouseWheelDown(TObject *Sender, TShiftState Shift, TPoint &MousePos,
          bool &Handled)
{
	scroll(20);
}
//---------------------------------------------------------------------------

void __fastcall TForm3::FormMouseWheelUp(TObject *Sender, TShiftState Shift, TPoint &MousePos,
          bool &Handled)
{
	scroll(-20);
}
//---------------------------------------------------------------------------

void __fastcall TForm3::Edit1Change(TObject *Sender)
{
	TEdit*	Edit	= dynamic_cast<TEdit*>(Sender);
	if(Edit){
		int t 			= Edit->SelStart;
		Edit->Text		= edit_control(Edit->Text);
		Edit->SelStart	= t;
		Edit->Color		= 14024703;
	}
	if(Edit == Edit1)	set_low(1000.0*Edit1->Text.ToDouble());
	if(Edit == Edit2)   set_hi(1000.0*Edit2->Text.ToDouble());
}
//---------------------------------------------------------------------------

void __fastcall TForm3::UpDown2ChangingEx(TObject *Sender, bool &AllowChange, int NewValue,
          TUpDownDirection Direction)
{
 	if(Direction == updUp){
		set_low(_low + 100);
	}else{
		set_low(_low - 100);
	}
	Edit1->Text	= FloatToStr(int(_low/100)/10.0);
}
//---------------------------------------------------------------------------

void __fastcall TForm3::UpDown3ChangingEx(TObject *Sender, bool &AllowChange, int NewValue,
          TUpDownDirection Direction)
{
	if(Direction == updUp){
		set_hi(_hi + 100);
	}else{
		set_hi(_hi - 100);
	}
	Edit2->Text	= FloatToStr(int(_hi/100)/10.0);
}
//---------------------------------------------------------------------------

void __fastcall TForm3::CheckBox1Click(TObject *Sender)
{
	bool res   = analizator->set_relay(!CheckBox1->Checked);
	if(CheckBox1->Checked){
		set_hi(15000);
		set_low(9000);
	}else{
		set_low(400);
		set_hi(2400);
	}
	Edit1->Text	= FloatToStr(int(_low/100)/10.0);
	Edit2->Text	= FloatToStr(int(_hi/100)/10.0);


	if(res){
		if(CheckBox1->Checked) 	console<<"-- ����� +27�"<<endl;
		else                    console<<"-- ����� +12�"<<endl;
	}else{
		console<<"������ ��������� � ����������"<<endl;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm3::Button4Click(TObject *Sender)
{
	analizator->Level[0]	= _low;
	bool res	= analizator->set_level(0,_low);
	if(res){
		console<<"-- ������ ������� ��������������: " + FloatToStr(int(_low/100)/10.0) + "�"<<endl;
	}else{
		console<<"������ ��������� � ����������"<<endl;
	}
	analizator->Level[1]	= _hi;
	res	= analizator->set_level(1,_hi);
	if(res){
		console<<"-- ������� ������� ��������������: " + FloatToStr(int(_hi/100)/10.0) + "�"<<endl;
	}else{
		console<<"������ ��������� � ����������"<<endl;
	}

}
//---------------------------------------------------------------------------

void __fastcall TForm3::ToolButton4Click(TObject *Sender)
{
	stop	= true;
}
//---------------------------------------------------------------------------

void __fastcall TForm3::FormResize(TObject *Sender)
{
	String a = "asdfjsadfsdaf";
    Label1->Caption	= minimize_text(a, Canvas, Label1->Width);
}
//---------------------------------------------------------------------------


void __fastcall TForm3::FormClose(TObject *Sender, TCloseAction &Action)
{
	psu->close_dll();
	mgqs->disconnect();
	mgqs->close_dll();
	save("Default.scr");
}
//---------------------------------------------------------------------------

void __fastcall TForm3::Button5Click(TObject *Sender)
{
	if (!psu->is_connected()){
			connect_psu();
			psu->set_voltage(1, 5.0);
			psu->enable(1);
			double volt = 0;
			Sleep(200);
			psu->get_voltage(1, volt);
			console<<FloatToStr(volt)<<" �"<<endl;
	}else							disconnect_psu();
}
//---------------------------------------------------------------------------

void __fastcall TForm3::Button1Click(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------

void __fastcall TForm3::Button6Click(TObject *Sender)
{
	ListBox2->Items->SaveToFile("log.txt");
}
//---------------------------------------------------------------------------

void __fastcall TForm3::Button7Click(TObject *Sender)
{
	String filename = "logic_calibration_table.txt";
	calibrate->load_from_file(filename);
	if(FileExists(filename))	Memo3->Lines->LoadFromFile(filename);

	vector<pair<i32, i32> > vect = load_vect(ComboBox1->ItemIndex, CheckBox2->Checked);
	show_vect(vect);
//	calibrate->save_to_file("111.txt");
}
//---------------------------------------------------------------------------

void __fastcall TForm3::Button8Click(TObject *Sender)
{

	vector<pair<i32, i32> > vect = load_vect(ComboBox1->ItemIndex, CheckBox2->Checked);
	if(vect.size() == 0){
		ShowMessage("��� ������ ��� ��������");
		return;
	}

	u8 mode 	= ComboBox1->ItemIndex;
	i16 min 	= vect[0].first;
	i16 max 	= vect[vect.size() - 1].first;
	u16 count	= vect.size();
	analizator->setTableCount(mode,min,max,count);

}
//---------------------------------------------------------------------------

void __fastcall TForm3::CheckBox2Click(TObject *Sender)
{
	vector<pair<i32, i32> > vect = load_vect(ComboBox1->ItemIndex, CheckBox2->Checked);
	show_vect(vect);
}
//---------------------------------------------------------------------------

void __fastcall TForm3::ComboBox1Change(TObject *Sender)
{
	vector<pair<i32, i32> > vect = load_vect(ComboBox1->ItemIndex, CheckBox2->Checked);
	show_vect(vect);
}
//---------------------------------------------------------------------------

void __fastcall TForm3::Button9Click(TObject *Sender)
{
	vector<pair<i32, i32> > vect = load_vect(ComboBox1->ItemIndex, CheckBox2->Checked);
	if(vect.size() == 0 ){
		ShowMessage("��� ������ ��� ��������");
		return;
	}

	u8 mode 	= ComboBox1->ItemIndex;
	u16 offset 	= StrToInt(Edit5->Text);
	u16 count 	= StrToInt(Edit6->Text);
	u16* data	= new u16[count];
	u16 pack_size	= StrToInt(Edit7->Text);
	for(int  i = 0; i < count; i++){
		if(i + offset < vect.size()) data[i] = vect[i + offset].second;
		else data[i] = 0;
	}
	if(!analizator->setTablePacket(mode, offset, count, data,pack_size))	ShowMessage("������ �� ����������");
}
//---------------------------------------------------------------------------


