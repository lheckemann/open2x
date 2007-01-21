// Program to generate a  column parity table for yaffs_ecc.c



// Generate a column parity byte for a byte.
// This byte is already aligned for the ecc.
// The output byte has the form:
// bit 7: p4
// bit 6: p4'
// bit 5: p2
// bit 4: p2'
// bit 3: p1
// bit 2: p1'
// bit 1: unused.
// bit 0 of the output byte holds the line parity.
unsigned char entry(unsigned char x)
{
	unsigned char b0, b1, b2, b3, b4, b5, b6, b7;
	unsigned char p4, p2, p1, p4p, p2p, p1p;
	unsigned char linep;
	unsigned char result;
	
	b0 = (x & 0x01) ? 1 : 0;
	b1 = (x & 0x02) ? 1 : 0;
	b2 = (x & 0x04) ? 1 : 0;
	b3 = (x & 0x08) ? 1 : 0;
	b4 = (x & 0x10) ? 1 : 0;
	b5 = (x & 0x20) ? 1 : 0;
	b6 = (x & 0x40) ? 1 : 0;
	b7 = (x & 0x80) ? 1 : 0;
	
	p4 = b7 ^ b6 ^ b5 ^ b4;     p4p = b3 ^ b2 ^ b1 ^ b0;
	p2 = b7 ^ b6 ^ b3 ^ b2;     p2p = b5 ^ b4 ^ b1 ^ b0;
	p1 = b7 ^ b5 ^ b3 ^ b1;     p1p = b6 ^ b4 ^ b2 ^ b0;
	
	linep = p1 ^ p1p;
	
	result = 0;
	if(p4)    result |= 0x80;
	if(p4p)   result |= 0x40;
	if(p2)    result |= 0x20;
	if(p2p)   result |= 0x10;
	if(p1)    result |= 0x08;
	if(p1p)   result |= 0x04;
	if(linep) result |= 0x01;
	
	//result >>= 2;
	//if(linep) result |= 0x40;
	
	return result;
	
}


int main(int argc, char *argv[])
{
	unsigned i;
	
	printf("const unsigned char column_parity_table[] = {");
	for(i = 0; i < 256; i++)
	{
		if((i & 0xf) == 0) printf("\n");
		printf("0x%02x, ",entry((unsigned char) i));
	}
	printf("\n};\n");
}


