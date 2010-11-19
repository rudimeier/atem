#include <stdint.h>

//---------------------------------------------------------------
// itoas() - iMalc version updated ver. 0.8
//---------------------------------------------------------------
int itoa_int32( char *s, int32_t snum )
{
	char *ps = s;
	uint32_t num1 = snum, num2, num3, div;
	if (snum < 0) {
		*ps++ = '-';
		num1 = -snum;
	}
	if (num1 < 10000) {
		if (num1 < 10) goto L1;
		if (num1 < 100) goto L2;
		if (num1 < 1000) goto L3;
	} else {
		num2 = num1 / 10000;
		num1 -= num2 * 10000;
		if (num2 < 10000) {
			if (num2 < 10) goto L5;
			if (num2 < 100) goto L6;
			if (num2 < 1000) goto L7;
		} else {
			num3 = num2 / 10000;
			num2 -= num3 * 10000;
			if (num3 >= 10) {
				*ps++ = '0' + (char)(div = (num3*6554UL)>>16);
				num3 -= div*10;
			}
			*ps++ = '0' + (char)(num3);
		}
		*ps++ = '0' + (char)(div = (num2*8389UL)>>23);
		num2 -= div*1000;
L7:
		*ps++ = '0' + (char)(div = (num2*5243UL)>>19);
		num2 -= div*100;
L6:
		*ps++ = '0' + (char)(div = (num2*6554UL)>>16);
		num2 -= div*10;
L5:
		*ps++ = '0' + (char)(num2);
    }
	*ps++ = '0' + (char)(div = (num1*8389UL)>>23);
	num1 -= div*1000;
L3:
	*ps++ = '0' + (char)(div = (num1*5243UL)>>19);
	num1 -= div*100;
L2:
	*ps++ = '0' + (char)(div = (num1*6554UL)>>16);
	num1 -= div*10;
L1:
	*ps++ = '0' + (char)(num1);
	
	return ps - s;
}




int itoa_int64( char *s, int64_t snum )
{
	char *ps = s;
	uint64_t num1 = snum, num2, num3, num4, num5, div;
	
	if (snum < 0) {
		*ps++ = '-';
		num1 = -snum;
	}
	
	if (num1 < 10000) {
		if (num1 < 10) goto L1;
		if (num1 < 100) goto L2;
		if (num1 < 1000) goto L3;
	} else {
		num2 = num1 / 10000;
		num1 -= num2 * 10000;
		if (num2 < 10000) {
			if (num2 < 10) goto L5;
			if (num2 < 100) goto L6;
			if (num2 < 1000) goto L7;
		} else {
			num3 = num2 / 10000;
			num2 -= num3 * 10000;
			if (num3 < 10000) {
				if (num3 < 10) goto L9;
				if (num3 < 100) goto L10;
				if (num3 < 1000) goto L11;
			} else {
				num4 = num3 / 10000;
				num3 -= num4 * 10000;
				if (num4 < 10000) {
					if (num4 < 10) goto L13;
					if (num4 < 100) goto L14;
					if (num4 < 1000) goto L15;
				} else {
					num5 = num4 / 10000;
					num4 -= num5 * 10000;
					if (num5 < 10000) {
						if (num5 < 10) goto L17;
						if (num5 < 100) goto L18;
					}
					*ps++ = '0' + (char)(div = (num5*5243UL)>>19);
					num5 -= div*100;
L18:
					*ps++ = '0' + (char)(div = (num5*6554UL)>>16);
					num5 -= div*10;
L17:
					*ps++ = '0' + (char)(num5);
				}
				*ps++ = '0' + (char)(div = (num4*8389UL)>>23);
				num4 -= div*1000;
L15:
				*ps++ = '0' + (char)(div = (num4*5243UL)>>19);
				num4 -= div*100;
L14:
				*ps++ = '0' + (char)(div = (num4*6554UL)>>16);
				num4 -= div*10;
L13:
				*ps++ = '0' + (char)(num4);
			}
			*ps++ = '0' + (char)(div = (num3*8389UL)>>23);
			num3 -= div*1000;
L11:
			*ps++ = '0' + (char)(div = (num3*5243UL)>>19);
			num3 -= div*100;
L10:
			*ps++ = '0' + (char)(div = (num3*6554UL)>>16);
			num3 -= div*10;
L9:
			*ps++ = '0' + (char)(num3);
		}
		*ps++ = '0' + (char)(div = (num2*8389UL)>>23);
		num2 -= div*1000;
L7:
		*ps++ = '0' + (char)(div = (num2*5243UL)>>19);
		num2 -= div*100;
L6:
		*ps++ = '0' + (char)(div = (num2*6554UL)>>16);
		num2 -= div*10;
L5:
		*ps++ = '0' + (char)(num2);
    }
	*ps++ = '0' + (char)(div = (num1*8389UL)>>23);
	num1 -= div*1000;
L3:
	*ps++ = '0' + (char)(div = (num1*5243UL)>>19);
	num1 -= div*100;
L2:
	*ps++ = '0' + (char)(div = (num1*6554UL)>>16);
	num1 -= div*10;
L1:
	*ps++ = '0' + (char)(num1);
	
	return ps - s;

}




int itoa_uint64( char *s, uint64_t snum )
{
	char *ps = s;
	uint64_t num1 = snum, num2, num3, num4, num5, div;
	
	if (num1 < 10000) {
		if (num1 < 10) goto L1;
		if (num1 < 100) goto L2;
		if (num1 < 1000) goto L3;
	} else {
		num2 = num1 / 10000;
		num1 -= num2 * 10000;
		if (num2 < 10000) {
			if (num2 < 10) goto L5;
			if (num2 < 100) goto L6;
			if (num2 < 1000) goto L7;
		} else {
			num3 = num2 / 10000;
			num2 -= num3 * 10000;
			if (num3 < 10000) {
				if (num3 < 10) goto L9;
				if (num3 < 100) goto L10;
				if (num3 < 1000) goto L11;
			} else {
				num4 = num3 / 10000;
				num3 -= num4 * 10000;
				if (num4 < 10000) {
					if (num4 < 10) goto L13;
					if (num4 < 100) goto L14;
					if (num4 < 1000) goto L15;
				} else {
					num5 = num4 / 10000;
					num4 -= num5 * 10000;
					if (num5 < 10000) {
						if (num5 < 10) goto L17;
						if (num5 < 100) goto L18;
						if (num5 < 1000) goto L19;
					}
					
					*ps++ = '0' + (char)(div = (num5*8389UL)>>23);
					num5 -= div*1000;
L19:
					*ps++ = '0' + (char)(div = (num5*5243UL)>>19);
					num5 -= div*100;
L18:
					*ps++ = '0' + (char)(div = (num5*6554UL)>>16);
					num5 -= div*10;
L17:
					*ps++ = '0' + (char)(num5);
				}
				*ps++ = '0' + (char)(div = (num4*8389UL)>>23);
				num4 -= div*1000;
L15:
				*ps++ = '0' + (char)(div = (num4*5243UL)>>19);
				num4 -= div*100;
L14:
				*ps++ = '0' + (char)(div = (num4*6554UL)>>16);
				num4 -= div*10;
L13:
				*ps++ = '0' + (char)(num4);
			}
			*ps++ = '0' + (char)(div = (num3*8389UL)>>23);
			num3 -= div*1000;
L11:
			*ps++ = '0' + (char)(div = (num3*5243UL)>>19);
			num3 -= div*100;
L10:
			*ps++ = '0' + (char)(div = (num3*6554UL)>>16);
			num3 -= div*10;
L9:
			*ps++ = '0' + (char)(num3);
		}
		*ps++ = '0' + (char)(div = (num2*8389UL)>>23);
		num2 -= div*1000;
L7:
		*ps++ = '0' + (char)(div = (num2*5243UL)>>19);
		num2 -= div*100;
L6:
		*ps++ = '0' + (char)(div = (num2*6554UL)>>16);
		num2 -= div*10;
L5:
		*ps++ = '0' + (char)(num2);
    }
	*ps++ = '0' + (char)(div = (num1*8389UL)>>23);
	num1 -= div*1000;
L3:
	*ps++ = '0' + (char)(div = (num1*5243UL)>>19);
	num1 -= div*100;
L2:
	*ps++ = '0' + (char)(div = (num1*6554UL)>>16);
	num1 -= div*10;
L1:
	*ps++ = '0' + (char)(num1);
	
	return ps - s;

}



