#include "keyboard_map.h"
/* there are 25 lines each of 80 columns; each element takes 2 bytes */
#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE * LINES

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08

#define ENTER_KEY_CODE 0x1C
#define BACKSPACE_KEY_CODE 0x0E
#define ADDITION_KEY_CODE 0x3B
#define MULTIPLICATION_KEY_CODE 0x3D 
#define SUBTRACTION_KEY_CODE 0x3C
#define DIVISION_KEY_CODE 0x3E
#define MODULO_KEY_CODE 0x3F
extern unsigned char keyboard_map[128];
extern void keyboard_handler(void);
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);
extern void load_idt(unsigned long *idt_ptr);
/* current cursor location */
unsigned int current_loc = 0;
/* video memory begins at address 0xb8000 */
char *vidptr = (char*)0xb8000;
/*backspace counter */
unsigned int backspace_count = 0;
/*chars on current row*/
char charsOnRow[64];
int countOnRow = 0;
unsigned int system_init = 0;
/*Array filled with commands*/
unsigned char commands[1][25] = {'c','l','e','a','r','#'};
struct IDT_entry {
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];

struct file_system{
unsigned char storage[128];
unsigned char name[10];
};
void idt_init(void)
{
	unsigned long keyboard_address;
	unsigned long idt_address;
	unsigned long idt_ptr[2];

	/* populate IDT entry of keyboard's interrupt */
	keyboard_address = (unsigned long)keyboard_handler;
	IDT[0x21].offset_lowerbits = keyboard_address & 0xffff;
	IDT[0x21].selector = KERNEL_CODE_SEGMENT_OFFSET;
	IDT[0x21].zero = 0;
	IDT[0x21].type_attr = INTERRUPT_GATE;
	IDT[0x21].offset_higherbits = (keyboard_address & 0xffff0000) >> 16;

	/*     Ports
	*	 PIC1	PIC2
	*Command 0x20	0xA0
	*Data	 0x21	0xA1
	*/

	/* ICW1 - begin initialization */
	write_port(0x20 , 0x11);
	write_port(0xA0 , 0x11);

	/* ICW2 - remap offset address of IDT */
	/*
	* In x86 protected mode, we have to remap the PICs beyond 0x20 because
	* Intel have designated the first 32 interrupts as "reserved" for cpu exceptions
	*/
	write_port(0x21 , 0x20);
	write_port(0xA1 , 0x28);

	/* ICW3 - setup cascading */
	write_port(0x21 , 0x00);
	write_port(0xA1 , 0x00);

	/* ICW4 - environment info */
	write_port(0x21 , 0x01);
	write_port(0xA1 , 0x01);
	/* Initialization finished */

	/* mask interrupts */
	write_port(0x21 , 0xff);
	write_port(0xA1 , 0xff);

	/* fill the IDT descriptor */
	idt_address = (unsigned long)IDT ;
	idt_ptr[0] = (sizeof (struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16 ;

	load_idt(idt_ptr);
}
int compare(char list[], const int size){//c l e a r, 5
int foundMatch = 0;
int count = 0;
int i = 0;
for(i = 0; i < 1; i++){
	for(int j = 0; j < size; j++){
	if(list[j] == commands[i][j])
		count++;
	else
		break;
		
		}
	}
if(count == size && commands[i - 1][count] == '#')
	foundMatch = 1;
else
	foundMatch = 0;
return foundMatch;
}

void kb_init(void)
{
	/* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
	write_port(0x21 , 0xFD);
}

void kprint(const char *str)
{
	unsigned int i = 0;
	while (str[i] != '\0') {
		vidptr[current_loc++] = str[i++];
		vidptr[current_loc++] = 0x07;
	}
}

void charPrint(char list[], int size)
{
 unsigned int i = 0;
	while (i < size) {
		vidptr[current_loc++] = list[i++];
		vidptr[current_loc++] = 0x07;		
	}
}
char getKeyValue(int value){
	if(value == 1)
		return keyboard_map[0x02];
	else if(value == 2)
		return keyboard_map[0x03];
	else if(value == 3)
		return keyboard_map[0x04];
	else if(value == 4)
		return keyboard_map[0x05];
	else if(value == 5)
		return keyboard_map[0x06];
	else if(value == 6)
		return keyboard_map[0x07];
	else if(value == 7)	
		return keyboard_map[0x08];
	else if(value == 8)
		return keyboard_map[0x09];
	else if(value == 9)
		return keyboard_map[0x0A];
	else if(value == 0)
		return keyboard_map[0x0B];
	else
		return '#';
}


void divide(char list[], const int size)//[9, 3, ,1,3,#], 4
{
	int sizeOfFirst = -1; //1
	int sizeOfSecond = 0; //1
	int numOne = 0;
	int numTwo = 0;
	unsigned int i = 0;
	while(i < size)	{
	if(list[i] == ' ' && sizeOfFirst == -1 || list[i] == '#' && sizeOfFirst == -1){
		sizeOfFirst = i;
		i++;
		continue;
		}

	if(list[i] == '#' && sizeOfFirst > -1){
		sizeOfSecond = i - sizeOfFirst - 1;
		}
	i++;	
	}
	int count = 0;
	for(i = sizeOfFirst; i > 0; i--){
		int ten = 10;
		for(int j = 0; j <= i; j++)
			{
				if(j == 1){
					ten = 1;
				}	
				if(j == 2){
					ten = 10;					
				}
				if(j >= 3){
					ten *= 10;					
				}
				if(j == i){
					numOne += (list[count] - '0') * ten;		
					count++;
				}	
			}
	}
	count = size - sizeOfSecond - 1;	
	for(i = sizeOfSecond; i > 0; i--){	
		int ten = 10;
		for(int j = 0; j <= i; j++)
			{
				if(j == 1){
					ten = 1;
				}	
				if(j == 2){
					ten = 10;					
				}
				if(j >= 3){
					ten *= 10;					
				}
				if(j == i){
					numTwo += (list[count] - '0') * ten;		
					count++;
				}	
			}
	}
	int answer = 0;	
	if(numOne > numTwo)
		answer = numOne / numTwo;
	else
		answer = numTwo / numOne;	
	
	count = 0;
	if(answer != 0){	
		while(answer > 0){			
			list[count] = getKeyValue(answer % 10);		
			answer = answer / 10;
			count++;	
		}
	}
	else{
		list[0] = '0';
		count = 1;

	}
	for(int i = 0; i < count / 2; i++)
		{
		char temp = list[i];
		list[i] = list[count - i - 1];
		list[count - i - 1] = temp;
		}
	charPrint(list, count);
}

void subtract(char list[], const int size)//[9, 3, ,1,3,#], 4
{
	int sizeOfFirst = -1; //1
	int sizeOfSecond = 0; //1
	int numOne = 0;
	int numTwo = 0;
	unsigned int i = 0;
	while(i < size)	{
	if(list[i] == ' ' && sizeOfFirst == -1 || list[i] == '#' && sizeOfFirst == -1){
		sizeOfFirst = i;
		i++;
		continue;
		}

	if(list[i] == '#' && sizeOfFirst > -1){
		sizeOfSecond = i - sizeOfFirst - 1;
		}
	i++;	
	}
	int count = 0;
	for(i = sizeOfFirst; i > 0; i--){
		int ten = 10;
		for(int j = 0; j <= i; j++)
			{
				if(j == 1){
					ten = 1;
				}	
				if(j == 2){
					ten = 10;					
				}
				if(j >= 3){
					ten *= 10;					
				}
				if(j == i){
					numOne += (list[count] - '0') * ten;		
					count++;
				}	
			}
	}
	count = size - sizeOfSecond - 1;	
	for(i = sizeOfSecond; i > 0; i--){	
		int ten = 10;
		for(int j = 0; j <= i; j++)
			{
				if(j == 1){
					ten = 1;
				}	
				if(j == 2){
					ten = 10;					
				}
				if(j >= 3){
					ten *= 10;					
				}
				if(j == i){
					numTwo += (list[count] - '0') * ten;		
					count++;
				}	
			}
	}
	int answer = 0;	
	if(numOne > numTwo)
		answer = numOne - numTwo;
	else
		answer = numTwo - numOne;	
	
	count = 0;
	if(answer != 0){	
		while(answer > 0){			
			list[count] = getKeyValue(answer % 10);		
			answer = answer / 10;
			count++;	
		}
	}
	else{
		list[0] = '0';
		count = 1;

	}
	for(int i = 0; i < count / 2; i++)
		{
		char temp = list[i];
		list[i] = list[count - i - 1];
		list[count - i - 1] = temp;
		}
	charPrint(list, count);
}


void modulo(char list[], const int size)//[9, 3, ,1,3,#], 4
{
	int sizeOfFirst = -1; //1
	int sizeOfSecond = 0; //1
	int numOne = 0;
	int numTwo = 0;
	unsigned int i = 0;
	while(i < size)	{
	if(list[i] == ' ' && sizeOfFirst == -1 || list[i] == '#' && sizeOfFirst == -1){
		sizeOfFirst = i;
		i++;
		continue;
		}

	if(list[i] == '#' && sizeOfFirst > -1){
		sizeOfSecond = i - sizeOfFirst - 1;
		}
	i++;	
	}
	int count = 0;
	for(i = sizeOfFirst; i > 0; i--){
		int ten = 10;
		for(int j = 0; j <= i; j++)
			{
				if(j == 1){
					ten = 1;
				}	
				if(j == 2){
					ten = 10;					
				}
				if(j >= 3){
					ten *= 10;					
				}
				if(j == i){
					numOne += (list[count] - '0') * ten;		
					count++;
				}	
			}
	}
	count = size - sizeOfSecond - 1;	
	for(i = sizeOfSecond; i > 0; i--){	
		int ten = 10;
		for(int j = 0; j <= i; j++)
			{
				if(j == 1){
					ten = 1;
				}	
				if(j == 2){
					ten = 10;					
				}
				if(j >= 3){
					ten *= 10;					
				}
				if(j == i){
					numTwo += (list[count] - '0') * ten;		
					count++;
				}	
			}
	}
	int answer = numOne % numTwo;	
	
	count = 0;
	if(answer != 0){	
		while(answer > 0){			
			list[count] = getKeyValue(answer % 10);		
			answer = answer / 10;
			count++;	
		}
	}
	else{
		list[0] = '0';
		count = 1;

	}
	for(int i = 0; i < count / 2; i++)
		{
		char temp = list[i];
		list[i] = list[count - i - 1];
		list[count - i - 1] = temp;
		}
	charPrint(list, count);
}

void multiply(char list[], const int size)//[9, 3, ,1,3,#], 4
{
	int sizeOfFirst = -1; //1
	int sizeOfSecond = 0; //1
	int numOne = 0;
	int numTwo = 0;
	unsigned int i = 0;
	while(i < size)	{
	if(list[i] == ' ' && sizeOfFirst == -1 || list[i] == '#' && sizeOfFirst == -1){
		sizeOfFirst = i;
		i++;
		continue;
		}

	if(list[i] == '#' && sizeOfFirst > -1){
		sizeOfSecond = i - sizeOfFirst - 1;
		}
	i++;	
	}
	int count = 0;
	for(i = sizeOfFirst; i > 0; i--){
		int ten = 10;
		for(int j = 0; j <= i; j++)
			{
				if(j == 1){
					ten = 1;
				}	
				if(j == 2){
					ten = 10;					
				}
				if(j >= 3){
					ten *= 10;					
				}
				if(j == i){
					numOne += (list[count] - '0') * ten;		
					count++;
				}	
			}
	}
	count = size - sizeOfSecond - 1;	
	for(i = sizeOfSecond; i > 0; i--){	
		int ten = 10;
		for(int j = 0; j <= i; j++)
			{
				if(j == 1){
					ten = 1;
				}	
				if(j == 2){
					ten = 10;					
				}
				if(j >= 3){
					ten *= 10;					
				}
				if(j == i){
					numTwo += (list[count] - '0') * ten;		
					count++;
				}	
			}
	}
	int answer = numOne * numTwo;	
	
	count = 0;
	while(answer > 0){			
		list[count] = getKeyValue(answer % 10);		
		answer = answer / 10;
		count++;	
	}
	for(int i = 0; i < count / 2; i++)
		{
		char temp = list[i];
		list[i] = list[count - i - 1];
		list[count - i - 1] = temp;
		}
	charPrint(list, count);
}

void add(char list[], const int size)//[9, 3, ,1,3,#], 4
{
	int sizeOfFirst = -1; //size of the first number
	int sizeOfSecond = 0; //size of the second number
	int numOne = 0; //what the first number is
	int numTwo = 0; //what the second number is
	unsigned int i = 0;
	while(i < size)	{ //this loop gets the size of the first number and the second number
	if(list[i] == ' ' && sizeOfFirst == -1 || list[i] == '#' && sizeOfFirst == -1){
		sizeOfFirst = i;
		i++;
		continue;
		}

	if(list[i] == '#' && sizeOfFirst > -1){
		sizeOfSecond = i - sizeOfFirst - 1;
		
		}
	i++;	
	}
	int count = 0;
	/*
	Since we can't use itoa, we have to break the number up, and then multiply it by 10 to the
	so if our number is 93 it will take it and go 9 * 10 = 90 3 * 1 = 3 90+3 = 93
	it works for an infinietely large number, so 12345 would go 1*10000 2 * 1000 3 * 100 and so 		on...
	*/	
	for(i = sizeOfFirst; i > 0; i--){ 
		int ten = 10;
		for(int j = 0; j <= i; j++)
			{
				if(j == 1){
					ten = 1;
				}	
				if(j == 2){
					ten = 10;					
				}
				if(j >= 3){
					ten *= 10;					
				}
				if(j == i){
					numOne += (list[count] - '0') * ten;		
					count++;
				}
				
			}

	}
	count = size - sizeOfSecond - 1;	
	/*
	This is the exact same as the above for loop but take note of count
	Since we use a '#' to determine when our array is over, we need to account for that
	This for loop works by finding the starting number of the second number, so if we have
	[0, 1, 3, ' ', 8, 6, #] it will start at 8, multiple that by 10, and then move to six, and 		multiply that by 1, so we end up with 86. It then stops at the #.
	*/	
	for(i = sizeOfSecond; i > 0; i--){	
		int ten = 10;
		for(int j = 0; j <= i; j++)
			{
				if(j == 1){
					ten = 1;
				}	
				if(j == 2){
					ten = 10;					
				}
				if(j >= 3){
					ten *= 10;					
				}
				if(j == i){
					numTwo += (list[count] - '0') * ten;		
					count++;
				}
				
			}
	}
	int answer = numOne + numTwo; //takes our two answers from above for loop and adds them
	
	count = 0;
	/* 
	Bread and butter of being able to display the results. This will take our answer, and chop it 		back up into chars so it can be stored back into a char array and be printed by the function 		charPrint. It stores the numbers in backwards, so the for loop underneath reverses the order 		so we can get the correct answer.		
	*/	
	while(answer > 0){			
		list[count] = getKeyValue(answer % 10);		
		answer = answer / 10;
		count++;
		
	}
	for(int i = 0; i < count / 2; i++)
		{
		char temp = list[i];
		list[i] = list[count - i - 1];
		list[count - i - 1] = temp;
		}	
	charPrint(list, count);
}

void clear_screen(void)
{
	unsigned int i = 0;
	while (i < SCREENSIZE) {
		vidptr[i++] = ' ';
		vidptr[i++] = 0x07;
	}
current_loc = 0;
}

void kprint_newline(void)
{
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	current_loc = current_loc + (line_size - current_loc % (line_size));
	vidptr[current_loc++] = '>';
	current_loc++;
	if(compare(charsOnRow, countOnRow) == 1 && system_init == 1)
		clear_screen();
	for(int i = 0; i < 64; i++)
		charsOnRow[i] = '#';
	countOnRow = 0;
}
void kprint_newline_noclear(void){
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	current_loc = current_loc + (line_size - current_loc % (line_size));
}

void kprint_rmvLetter(void){
	current_loc = current_loc - 2;	
	vidptr[current_loc] = 0x07;
	vidptr[current_loc] = ' ';
		if(countOnRow > 0){        
		charsOnRow[countOnRow] = '#';
        	countOnRow--; 
		}
}

void keyboard_handler_main(void)
{
	unsigned char status;
	char keycode;

	/* write EOI */
	write_port(0x20, 0x20);

	status = read_port(KEYBOARD_STATUS_PORT);
	/* Lowest bit of status will be set if buffer is not empty */
	if (status & 0x01) {
		keycode = read_port(KEYBOARD_DATA_PORT);
		if(keycode < 0)
			return;

		if(keycode == ENTER_KEY_CODE) {
			kprint_newline();
			return;
		}
		if(keycode == BACKSPACE_KEY_CODE){
			kprint_rmvLetter();
			return;		
		}
		if(keycode == ADDITION_KEY_CODE){
			charsOnRow[countOnRow++] = '#';
			kprint_newline_noclear();			
			add(charsOnRow, countOnRow); 
			kprint_newline();
			return;
		}
		if(keycode == MULTIPLICATION_KEY_CODE){			
			charsOnRow[countOnRow++] = '#';
			kprint_newline_noclear();
			multiply(charsOnRow, countOnRow); 
			kprint_newline();
			return;
			}
		if(keycode == SUBTRACTION_KEY_CODE){
			charsOnRow[countOnRow++] = '#';
			kprint_newline_noclear();			
			subtract(charsOnRow, countOnRow); 	
			kprint_newline();
			return;
			}
		if(keycode == DIVISION_KEY_CODE){
			charsOnRow[countOnRow++] = '#';
			kprint_newline_noclear();				
			divide(charsOnRow, countOnRow); 			
			kprint_newline();
			return;
			}
		if(keycode == MODULO_KEY_CODE){
			charsOnRow[countOnRow++] = '#';
			kprint_newline_noclear();			
			modulo(charsOnRow, countOnRow); 
			kprint_newline();
			return;
			}
		vidptr[current_loc++] = keyboard_map[(unsigned char) keycode];
		vidptr[current_loc++] = 0x07;
		charsOnRow[countOnRow] =  keyboard_map[(unsigned char) keycode]; 		
		countOnRow++;
	}
}

void kmain(void)
{
	const char *st = "Welcome to cheebOS";
	clear_screen();	
	kprint(st);
	kprint_newline();
	system_init = 1;
	idt_init();
	kb_init();

	while(1);
}
