#include<reg51.h>

#define LCD1602_DATAPINS P0 //发送接收数据和命令
#define WRITEADDR 0x90	   //写地址  定义器件地址
#define READADDR  0x91	   //读地址
#define bias_value 2 // 偏置电压

//--定义使用的IO口--//
sbit I2C_SCL = P2^1;//I2C时钟
sbit I2C_SDA = P2^0;//I2C数据
sbit LCD1602_RW=P2^5; //读写区分
sbit LCD1602_RS=P2^6; //区分命令和数据
sbit LCD1602_E=P2^7; //使能信号
sbit range=P1^0;

unsigned int adNum,flag;
float value,a;
//--声明全局变量--//
void LCD1602_Delay1ms(unsigned int c);   //误差 0us
void LCD1602_Init();//LCD1602初始化子程序
void LCD1602_WriteCom(unsigned char com);//LCD1602写入8位命令子函数
void LCD1602_WriteData(unsigned char dat);//LCD1602写入8位数据子函数

void I2C_Delay10us();
void I2C_Start();//起始信号
void I2C_Stop();//终止信号
unsigned char I2C_SendByte(unsigned char dat, unsigned char ack);//通过I2C发送一个字节
unsigned char I2C_ReadByte();//使用I2c读取一个字节

void Pcf8591SendByte(unsigned char channel);//写入一个控制命令
unsigned char Pcf8591ReadByte();//读取一个转换值
void Pcf8591DaConversion(unsigned char value);//PCF8591的输出端输出模拟量

void mv200display();//20mv量程显示
void v2display();//2v量程显示
void v20display();
void v200display();

void main(){
	LCD1602_Init();
	while(1){	
		Pcf8591SendByte(3); 
		value=Pcf8591ReadByte()*0.01953;  //转为电压值
		if(range==0){
			LCD1602_Delay1ms(5);
			if(range==0){
				while(range==0);
				flag++;
				if(flag==4)
					flag=0;
			}
		}
		switch(flag){
			case 0: 
				mv200display();
				break;
			case 1: 
				v2display();
				break;
			case 2:
				v20display();
				break;
			case 3:
				v200display();
				break;
			default: 
				break;
		}
	}			 
}

void LCD1602_Delay1ms(unsigned int c){   //误差 0us
	unsigned char a, b;
	for(; c>0; c--){
		for(b=199; b>0; b--){
			for(a=1; a>0; a--);
		}      
	}
    	
}

void LCD1602_WriteCom(unsigned char com){	  //写入命令
	LCD1602_E = 0;	 //使能清零
	LCD1602_RS = 0;	 //选择写入命令
	LCD1602_RW = 0;	 //选择写入

	LCD1602_DATAPINS = com;	//由于4位的接线是接到P0口的高四位，所以传送高四位不用改
	LCD1602_Delay1ms(1);

	LCD1602_E = 1;	 //写入时序
	LCD1602_Delay1ms(5);
	LCD1602_E = 0;

	LCD1602_DATAPINS = com << 4; //发送低四位
	LCD1602_Delay1ms(1);

	LCD1602_E = 1;	 //写入时序
	LCD1602_Delay1ms(5);
	LCD1602_E = 0;
}

void LCD1602_WriteData(unsigned char dat){			//写入数据
	LCD1602_E = 0;	  //使能清零
	LCD1602_RS = 1;	  //选择写入数据
	LCD1602_RW = 0;	  //选择写入

	LCD1602_DATAPINS = dat;	//由于4位的接线是接到P0口的高四位，所以传送高四位不用改
	LCD1602_Delay1ms(1);

	LCD1602_E = 1;	  //写入时序
	LCD1602_Delay1ms(5);
	LCD1602_E = 0;

	LCD1602_DATAPINS = dat << 4; //写入低四位
	LCD1602_Delay1ms(1);

	LCD1602_E = 1;	  //写入时序
	LCD1602_Delay1ms(5);
	LCD1602_E = 0;
}

void LCD1602_Init(){ //LCD初始化子程序
	LCD1602_WriteCom(0x32);	 //将8位总线转为4位总线
	LCD1602_WriteCom(0x28);	 //在四位线下的初始化
	LCD1602_WriteCom(0x0c);  //开显示不显示光标
	LCD1602_WriteCom(0x06);  //写一个指针加1
	LCD1602_WriteCom(0x01);  //清屏
	LCD1602_WriteCom(0x80);  //设置数据指针起点
}

void I2C_Delay10us(){
	unsigned char a, b;
	for(b=1; b>0; b--){
		for(a=2; a>0; a--);
	}
}

void I2C_Start(){//启动I2C 起始信号
	I2C_SDA = 1;
	I2C_Delay10us();
	I2C_SCL = 1;
	I2C_Delay10us();//建立时间是I2C_SDA保持时间>4.7us
	I2C_SDA = 0;
	I2C_Delay10us();//保持时间是>4us
	I2C_SCL = 0;			
	I2C_Delay10us();		
}

void I2C_Stop(){//停止I2C
	I2C_SDA = 0;
	I2C_Delay10us();
	I2C_SCL = 1;
	I2C_Delay10us();//建立时间大于4.7us
	I2C_SDA = 1;
	I2C_Delay10us();		
}

unsigned char I2C_SendByte(unsigned char dat, unsigned char ack){//将输入的一字节数据bt发送
	unsigned char a = 0, b = 0;//最大255，一个机器周期为1us，最大延时255us。
	for(a=0; a<8; a++){//要发送8位，从最高位开始
		I2C_SDA = dat >> 7;	 //起始信号之后I2C_SCL=0，所以可以直接改变I2C_SDA信号
		dat = dat << 1;
		I2C_Delay10us();
		I2C_SCL = 1;
		I2C_Delay10us();//建立时间>4.7us
		I2C_SCL = 0;
		I2C_Delay10us();//时间大于4us		
	}

	I2C_SDA = 1;
	I2C_Delay10us();
	I2C_SCL = 1;
	while(I2C_SDA && (ack == 1)){ //等待应答，也就是等待从设备把I2C_SDA拉低
		b++;
		if(b > 200){//如果超过200us没有应答发送失败，或者为非应答，表示接收结束
			I2C_SCL = 0;
			I2C_Delay10us();
			return 0;
		}
	}

	I2C_SCL = 0;
	I2C_Delay10us();
 	return 1;		
}

unsigned char I2C_ReadByte(){//读出一字节数据
	unsigned char a = 0,dat = 0;
	I2C_SDA = 1;			//起始和发送一个字节之后I2C_SCL都是0
	I2C_Delay10us();
	for(a=0; a<8; a++){ //接收8个字节
		I2C_SCL = 1;
		I2C_Delay10us();
		dat <<= 1;
		dat |= I2C_SDA;
		I2C_Delay10us();
		I2C_SCL = 0;
		I2C_Delay10us();
	}
	return dat;		
}

void Pcf8591SendByte(unsigned char channel){	
	I2C_Start();
	I2C_SendByte(WRITEADDR, 1);    //发送写器件地址
	I2C_SendByte(0x40|channel, 0); //发送控制寄存器
	I2C_Stop();
}

unsigned char Pcf8591ReadByte(){
	unsigned char dat;
	I2C_Start();
	I2C_SendByte(READADDR, 1);//发送读器件地址
	dat=I2C_ReadByte();    //读取数据
	I2C_Stop();            //结束总线
    return dat;
}

void Pcf8591DaConversion(unsigned char value){
	I2C_Start();
	I2C_SendByte(WRITEADDR, 1);//发送写器件地址
	I2C_SendByte(0x40, 1);     //开启DA写到控制寄存器
	I2C_SendByte(value, 0);    //发送转换数值
	I2C_Stop();	
}

void mv200display(){
	if(value>=bias_value&&value<=2.25){
		value=value-bias_value;
		adNum=value*10000;
		LCD1602_WriteCom(0x80);//第一行第一个字符的地址
		LCD1602_WriteData('+');
		LCD1602_WriteData('0'+adNum%10000/1000);
		LCD1602_WriteData('0'+adNum%1000/100);
		LCD1602_WriteData('0'+adNum%100/10);
		LCD1602_WriteData('.');
		LCD1602_WriteData('0'+adNum%10);
		LCD1602_WriteData('m');
		LCD1602_WriteData('V');
	}else if(value>=1.85&&value<bias_value){
	   	value=bias_value-value;
		adNum=value*10000;
		LCD1602_WriteCom(0x80);//第一行第一个字符的地址
		LCD1602_WriteData('-');
		LCD1602_WriteData('0'+adNum%10000/1000);
		LCD1602_WriteData('0'+adNum%1000/100);
		LCD1602_WriteData('0'+adNum%100/10);
		LCD1602_WriteData('.');
		LCD1602_WriteData('0'+adNum%10);
		LCD1602_WriteData('m');
		LCD1602_WriteData('V');
	}else{
		LCD1602_WriteCom(0x80);
		LCD1602_WriteData('E');
		LCD1602_WriteData('R');
		LCD1602_WriteData('R');
		LCD1602_WriteData('O');
		LCD1602_WriteData('R');
	}	
}

void v2display(){
	if(value>=bias_value&&value<bias_value-2){
		value=value-bias_value;
		adNum=value*1000;       
		LCD1602_WriteCom(0x80);
		LCD1602_WriteData('+');
		LCD1602_WriteData('0'+adNum%10000/1000);
		LCD1602_WriteData('.');
		LCD1602_WriteData('0'+adNum%1000/100);
		LCD1602_WriteData('0'+adNum%100/10);
		LCD1602_WriteData('0'+adNum%10);
		LCD1602_WriteData('V');
	}else if(value>=0&&value<bias_value){
		value=bias_value-value;
		adNum=value*1000;
		LCD1602_WriteCom(0x80);
		LCD1602_WriteData('-');
		LCD1602_WriteData('0'+adNum%10000/1000);
		LCD1602_WriteData('.');
		LCD1602_WriteData('0'+adNum%1000/100);
		LCD1602_WriteData('0'+adNum%100/10);
		LCD1602_WriteData('0'+adNum%10);
		LCD1602_WriteData('V');
	}else{
		LCD1602_WriteCom(0x80);
		LCD1602_WriteData('E');
		LCD1602_WriteData('R');
		LCD1602_WriteData('R');
		LCD1602_WriteData('O');
		LCD1602_WriteData('R');
	}	
}

void v20display(){
	if(value>=bias_value&&value<bias_value+2){
		value=value-bias_value;
		value=value*10;//10分压还原
		adNum=value*100;        
		LCD1602_WriteCom(0x80);
		LCD1602_WriteData('+');
		LCD1602_WriteData('0'+adNum%10000/1000);
		LCD1602_WriteData('0'+adNum%1000/100);
		LCD1602_WriteData('.');
		LCD1602_WriteData('0'+adNum%100/10);
		LCD1602_WriteData('0'+adNum%10);
		LCD1602_WriteData('V');
	}else if(value>=bias_value-2&&value<bias_value){
		value=bias_value-value;
		value=value*10;//10分压还原
		adNum=value*100;        
		LCD1602_WriteCom(0x80);
		LCD1602_WriteData('-');
		LCD1602_WriteData('0'+adNum%10000/1000);
		LCD1602_WriteData('0'+adNum%1000/100);
		LCD1602_WriteData('.');
		LCD1602_WriteData('0'+adNum%100/10);
		LCD1602_WriteData('0'+adNum%10);
		LCD1602_WriteData('V');
	}else{
		LCD1602_WriteCom(0x80);
		LCD1602_WriteData('E');
		LCD1602_WriteData('R');
		LCD1602_WriteData('R');
		LCD1602_WriteData('O');
		LCD1602_WriteData('R');
	}	
}
	
void v200display(){
	if(value>=bias_value&&value<bias_value+2){
		value=value-bias_value;
		value=value*100;//100分压还原
		adNum=value*10;        
		LCD1602_WriteCom(0x80);
		LCD1602_WriteData('+');
		LCD1602_WriteData('0'+adNum%10000/1000);
		LCD1602_WriteData('0'+adNum%1000/100);
		LCD1602_WriteData('0'+adNum%100/10);
		LCD1602_WriteData('.');
		LCD1602_WriteData('0'+adNum%10);
		LCD1602_WriteData('V');
	}else if(value>=bias_value-2&&value<bias_value){
		value=bias_value-value;
		value=value*100;//100分压还原
		adNum=value*10;        
		LCD1602_WriteCom(0x80);
		LCD1602_WriteData('-');
		LCD1602_WriteData('0'+adNum%10000/1000);
		LCD1602_WriteData('0'+adNum%1000/100);
		LCD1602_WriteData('0'+adNum%100/10);
		LCD1602_WriteData('.');
		LCD1602_WriteData('0'+adNum%10);
		LCD1602_WriteData('V');
	}else{
		LCD1602_WriteCom(0x80);
		LCD1602_WriteData('E');
		LCD1602_WriteData('R');
		LCD1602_WriteData('R');
		LCD1602_WriteData('O');
		LCD1602_WriteData('R');
	}	
}
