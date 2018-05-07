#include<iostream>						//for console input/output
#include<cstdlib>						//for atoi() function
#include<iomanip>						//for setw() and setfill()
#include<fstream>						//for file input/output
#include<sstream>						//for stringstream class (Standard Template Library)
#include<unordered_map>						//for unordered hashmap for symtab and optab (Standart Template Library)
#include<algorithm>						//for count() function
using namespace std;
/************************	CLASS ASSEMBLER		**************************************

data members:		type:			usage:
assembly_code		input file stream	corresponds to the input assembly code
listing			output file stream	corresponds to the assembly listing file
object_code		output file stream	corresponds to the output object code
object_code_name	string			name of the output object code file
assembly_code_name	string			name of the input assembly code file
program_name		string			name of the program specified in the assembly code
starting_address	string			address of first instruction
LOCCTR			int			location counter
program_length		int			length of the object program
OPTAB			unordered_map		hashmap that stores machine codes of mnemonic instructions
SYMTAB			unordered_map		hashmap that stores the address of symbols

member functions:		usage:
constructor			opens all the input and output files to use
				loads OPTAB from external file
pass()				one pass algorithm
read()				reads an input line from the assembly code file
destructor			closes all the input and output files
				prints all the files content

**********************************************************************************************/
class ASSEMBLER
{
	ifstream assembly_code;
	ofstream listing, object_code;
	string object_code_name, assembly_code_name, program_name, starting_address;
	int LOCCTR,program_length;
	struct OPCODE
	{
		public:
		string opcode;
	};
	unordered_map<string,OPCODE>OPTAB;
	struct SYMBOL
	{
		int address;
		bool defined;
		struct node
		{
			int address;
			node *link;
		}*linked_list;
	};
	unordered_map<string,SYMBOL>SYMTAB;
	public:
	ASSEMBLER(string, string);
	void pass();
	void read(string&, string&, string&);
	~ASSEMBLER();
};
int main(int argc, char* argv[])
{
	string output, input;
	if(argc==1)								//case : ./assembler
	{
		cout<<"PLEASE ENTER INPUT FILE NAME\n";
		cin>>input;
		cout<<"PLEASE ENTER OUPUT FILE NAME\n";
		cin>>output;
	}
	else if(argc==2)							//case : ./assembler input_file
	{
		input=argv[1];
		cout<<"PLEASE ENTER OUTPUT FILE NAME\n";
		cin>>output;
	}
	else									//case : ./assembler input_file output_file
	{
		input=argv[1];
		output=argv[2];
	}
	ASSEMBLER A(input,output);
	A.pass();
	return 0;
}
ASSEMBLER::ASSEMBLER(string input, string output)
{
	assembly_code_name=input;
	object_code_name=output;
	assembly_code.open(input);
	if(!assembly_code)
	{
		cout<<"ERROR: INPUT FILE DOES NOT EXIST\n";
		exit(0);
	}
	object_code.open(output);
	listing.open("assembly_listing.txt");
	ifstream opcodes("optab.txt");
	if(!opcodes)
	{
		cout<<"ERROR: OPCODES FILE IS MISSING\n";
		exit(0);
	}
	string name, code;
	while(opcodes)
	{
		OPCODE op;
		opcodes>>name>>code;
		op.opcode=code;
		OPTAB[name]=op;
	}
	opcodes.close();
}
void ASSEMBLER::pass()
{
	bool skip;
	string label, opcode, operand;
	stringstream init_text_record(""), text_record(""), machine_code("");
	int text_record_length, starting_address_int, current_address_int;
	read(label,opcode,operand);
	if(opcode=="START")
	{
		program_name=label;
		starting_address=operand;
		stringstream LOCCTR_init(starting_address);
		LOCCTR_init>>hex>>LOCCTR;
		starting_address_int=LOCCTR;
		listing<<setw(4)<<setfill('0')<<uppercase<<hex<<LOCCTR<<"\t"<<label<<"\t"<<opcode<<"\t"<<operand<<"\n";
		read(label,opcode,operand);

	}
	else
	{
		starting_address="0";
		LOCCTR=0;
	}
	object_code<<"H^"<<program_name;
	for(int i=0;i<6-starting_address.length();i++)
		object_code<<" ";
	object_code<<"^"<<setw(6)<<setfill('0')<<starting_address<<"^";
	long program_length_position=object_code.tellp();
	object_code<<"000000\n";
	init_text_record<<"T^"<<setw(6)<<setfill('0')<<uppercase<<hex<<LOCCTR<<"^";
	text_record.str("");
	while(opcode!="END")
	{
		current_address_int=LOCCTR;
		skip=false;
		machine_code.str("");
		if(label[0]!='.')
		{
			if(!label.empty())
			{
				if(SYMTAB.find(label)!=SYMTAB.end())
				{
					if(SYMTAB[label].defined==false)
					{
						string s=text_record.str();
			                        text_record_length=machine_code.str().length()+(text_record.str().length()-count(s.begin(),s.end(),'^'))/2;			
						if(text_record_length>0)
						{
							cout<<"THERE\n"<<init_text_record.str()<<setw(2)<<setfill('0')<<uppercase<<hex<<text_record_length<<text_record.str()<<"\n";
							object_code<<init_text_record.str()<<setw(2)<<setfill('0')<<uppercase<<hex<<text_record_length<<text_record.str()<<"\n";
							init_text_record.str("");
							text_record.str("");
						}
						SYMBOL::node *r,*q;
						for(r=SYMTAB[label].linked_list;r!=NULL;r=r->link)
							object_code<<"T^"<<setw(6)<<setfill('0')<<uppercase<<hex<<r->address<<"^02^"<<LOCCTR<<"\n";
						init_text_record.str("");
						init_text_record<<"T^"<<setw(6)<<setfill('0')<<uppercase<<hex<<LOCCTR<<"^";
						text_record.str("");
						for(r=SYMTAB[label].linked_list;r!=NULL;r=q)
						{
							q=r->link;
							delete r;
						}
						SYMTAB[label].address=LOCCTR;
						SYMTAB[label].defined=true;
						SYMTAB[label].linked_list=NULL;
					}
				}
				else
				{
					SYMBOL sym;
					sym.address=LOCCTR;
					sym.defined=true;
					sym.linked_list=NULL;
					SYMTAB[label]=sym;
				}
			}
			if(OPTAB.find(opcode)!=OPTAB.end())
			{
				machine_code<<OPTAB[opcode].opcode;
				if(opcode=="RSUB")
					machine_code<<"0000";
				else if(operand.substr(operand.length()-2,2)==",X")
				{
					string actual_operand=operand.substr(0,operand.length()-2);
					if(SYMTAB.find(actual_operand)!=SYMTAB.end())
					{
						if(SYMTAB[actual_operand].defined==false)
						{
							SYMBOL::node *temp, *r;
							temp=new SYMBOL::node;
							temp->address=LOCCTR+1;
							temp->link=NULL;
							for(r=SYMTAB[actual_operand].linked_list;r->link!=NULL;r=r->link);
							r->link=temp;
							machine_code<<"8000";
						}
						else
						{
							machine_code<<setw(4)<<setfill('0')<<uppercase<<hex<<SYMTAB[actual_operand].address+32768;
						}
					}
					else
					{
						SYMBOL sym;
						sym.address=0;
						sym.defined=false;
						sym.linked_list=new SYMBOL::node;
						sym.linked_list->address=LOCCTR+1;
						sym.linked_list->link=NULL;
						SYMTAB[actual_operand]=sym;
						machine_code<<"8000";
					}
				}
				else
				{
				
					if(SYMTAB.find(operand)!=SYMTAB.end())
					{
						if(SYMTAB[operand].defined==false)
						{
							SYMBOL::node *temp, *r;
							temp=new SYMBOL::node;
							temp->address=LOCCTR+1;
							temp->link=NULL;
							for(r=SYMTAB[operand].linked_list;r->link!=NULL;r=r->link);
							r->link=temp;
							machine_code<<"0000";
						}
						else
						{
							machine_code<<setw(4)<<setfill('0')<<uppercase<<hex<<SYMTAB[operand].address;
						}
					}
					else
					{
						SYMBOL sym;
						sym.address=0;
						sym.defined=false;
						sym.linked_list=new SYMBOL::node;
						sym.linked_list->address=LOCCTR+1;
						sym.linked_list->link=NULL;
						SYMTAB[operand]=sym;
						machine_code<<"0000";
					}
				}
				LOCCTR+=3;
			}
			else if(opcode=="WORD")
			{
				machine_code<<setw(6)<<setfill('0')<<uppercase<<hex<<atoi(operand.c_str());
				LOCCTR+=3;
			}
			else if(opcode=="BYTE")
			{
				
				if(operand[0]=='C')
				{
					for(int i=2;i<operand.length()-1;i++)
						machine_code<<uppercase<<hex<<(int)operand[i];
					LOCCTR+=operand.length()-3;
				}
				else
				{
					machine_code<<operand.substr(2,operand.length()-3);
					LOCCTR+=(operand.length()-3)/2;
				}
			}
			string s=text_record.str();
			text_record_length=(text_record.str().length()-count(s.begin(),s.end(),'^'))/2;
			if((opcode=="RESW") || (opcode=="RESB") || (text_record_length>30))
			{
				if(text_record_length>0)
				{
					cout<<"HERE\n"<<init_text_record.str()<<setw(2)<<setfill('0')<<uppercase<<hex<<text_record_length<<text_record.str()<<"\n";
					object_code<<init_text_record.str()<<setw(2)<<setfill('0')<<uppercase<<hex<<text_record_length<<text_record.str()<<"\n";
					init_text_record.str("");
					text_record.str("");
				}
				do
				{
					stringstream operand_string(operand);
					int operand_int;
					operand_string>>dec>>operand_int;
					if(opcode=="RESW")
						LOCCTR+=3*operand_int;
					else if(opcode=="RESB")
						LOCCTR+=operand_int;
					listing<<setw(4)<<setfill('0')<<uppercase<<hex<<current_address_int<<"\t"<<label<<"\t"<<opcode<<"\t"<<operand<<"\t"<<machine_code.str()<<"\n";
					current_address_int=LOCCTR;
					read(label,opcode,operand);
					if(SYMTAB.find(label)==SYMTAB.end())
					{
						SYMBOL sym;
	                       			sym.address=LOCCTR;
        	        		      	sym.defined=true;
                		  		sym.linked_list=NULL;
                      				SYMTAB[label]=sym;
					}
					skip=true;
				}while(opcode=="RESW" || opcode=="RESB");
				if(opcode!="END")
				{
					init_text_record.str("");
					init_text_record<<"T^"<<setw(6)<<setfill('0')<<uppercase<<hex<<LOCCTR<<"^";
					text_record.str("");
				}
			}
			if(machine_code.str().length()>0)
			{
				text_record<<"^"<<machine_code.str();
			}
		}
		if(!skip)
		{
			listing<<setw(4)<<setfill('0')<<uppercase<<hex<<current_address_int<<"\t"<<label<<"\t"<<opcode<<"\t"<<operand<<"\t"<<machine_code.str()<<"\n";
			read(label,opcode,operand);
		}
	}
	text_record_length+=machine_code.str().length()/2;
	object_code<<init_text_record.str()<<setw(2)<<setfill('0')<<uppercase<<hex<<text_record_length<<text_record.str()<<"\n";
	object_code<<"E^"<<setw(6)<<setfill('0')<<uppercase<<hex<<SYMTAB[operand].address;
	object_code.seekp(program_length_position);
	object_code<<setw(6)<<setfill('0')<<uppercase<<hex<<LOCCTR-starting_address_int<<"\n";
	listing<<"\t"<<label<<"\t"<<opcode<<"\t"<<operand<<"\n";
}
void ASSEMBLER::read(string &label,string &opcode,string &operand)
{
	getline(assembly_code,label,'\t');
	getline(assembly_code,opcode,'\t');
	getline(assembly_code,operand,'\n');
}
ASSEMBLER::~ASSEMBLER()
{
	assembly_code.close();
	listing.close();
	object_code.close();
	ifstream fin;
	string buffer;
	fin.open(assembly_code_name);
	getline(fin,buffer,'\0');
	cout<<"------   INPUT FILE   ------\n";
	cout<<buffer;
	cout<<"\n----------------------------\n\n";
	fin.close();
	fin.open("assembly_listing.txt");
	getline(fin,buffer,'\0');
	cout<<"----------------ASSEMBLY LISTING----------------\n";
	cout<<buffer;
	cout<<"\n------------------------------------------------\n\n";
	fin.close();
	fin.open(object_code_name);
	getline(fin,buffer,'\0');
	cout<<"--------------------------   OUTPUT FILE   --------------------------\n";
	cout<<buffer;
	cout<<"\n---------------------------------------------------------------------\n\n";
	fin.close();
}
