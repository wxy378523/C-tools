#include "Binary.h"

short 
Binary2Short(char * t_Bt){
    short s =(((short)(*(t_Bt + 0)) )<< 8 )+ (((short)(*(t_Bt +1 )))<< 0 );
    return s;
}

int
Short2Binary(int index,char * array,short t_Sh){
    char * El = array + index ;
    int i ;
    int offset ;
    for ( i = 0 ; i < 2 ;i++){
        offset = 16 - (i + 1) * 8 ;
        *(El + i) =(char)(( t_Sh >> offset ) & 0xff);
    }
    return 1;
}