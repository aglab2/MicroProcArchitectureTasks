#include "stdio.h"

#include "include/table_descriptor.h"
#include "include/entry_descriptor.h"

void print_binary( FILE* out, int number, int cnt){
	if (cnt) {
		print_binary(out, number >> 1, cnt - 1);
		fputc((number & 1) ? '1' : '0', out);
	}
}

void print_column(FILE *out, int number, int cnt, int pad){
	int i;
	fputc('|', out);
	for (i = 0; i < pad; i++) fputc(' ', out);
	print_binary(out, number, cnt);
	for (i = 0; i < pad; i++) fputc(' ', out);
}

void print_gdt(GDT_descriptor _gdt){
	FILE* output;
	int i;
	s_descriptor *gdt_base;

	uint32 base_address;
	uint32 segment_limit;

	int descriptor_count;

	output = fopen("GDT.TXT", "w");
	if (!output) {
		printf("Failed to open file!\n");
		return;
	}

	gdt_base = (s_descriptor *) _gdt.base;
	descriptor_count = (_gdt.limit+1) / sizeof(s_descriptor);

	fprintf(output, "------------Global Descriptor Table-------------\n");
	fprintf(output, "Index|    BASE    |  LIMIT  |G|D/B|L|AVL|P|DPL |S|Type\n");
	for (i = 0; i < descriptor_count; i++){ //number of elements = number of bytes / sizeof(element)
		s_descriptor cur_descr = gdt_base[i];

		base_address = (uint32) cur_descr.base0 + ((uint32) cur_descr.base16 << 16) + ((uint32) cur_descr.base24 << 24);	//32 bit
		segment_limit = (uint32) cur_descr.limit0 + ((uint32) cur_descr.limit16 << 16);				//20 bit
		fprintf(output, "%5d| 0x%08x | 0x%05x ", i, base_address, segment_limit);

		print_column(output, cur_descr.G, 1, 0);
		print_column(output, cur_descr.DB, 1, 1);
		print_column(output, cur_descr.L, 1, 0);
		print_column(output, cur_descr.AVL, 1, 1);
		print_column(output, cur_descr.P, 1, 0);
		print_column(output, cur_descr.DPL, 2, 1);
		print_column(output, cur_descr.S, 1, 0);
		print_column(output, cur_descr.type, 4, 0);
		fputc('\n', output);
	}
	fprintf(output, "------------------------------------------------\n");


	printf("GDT: base=0x%08x, limit=%d\n", _gdt.base, _gdt.limit+1);
	fclose(output);
}


void print_idt(IDT_descriptor _idt){
	FILE* output;
	int i;
	g_descriptor *idt_base;

	uint32 offset;
	uint16 selector;

	int descriptor_count;

	output = fopen("IDT.TXT", "w");
	if (!output) {
		printf("Failed to open file!\n");
		return;
	}

	idt_base = (g_descriptor *) _idt.base;
	descriptor_count = (_idt.limit+1) / sizeof(g_descriptor);

	fprintf(output, "---Interrupt Descriptor Table---\n");
	fprintf(output, "   OFFSET   |SELECTOR|P|DPL |Type\n");
	for (i = 0; i < descriptor_count; i++){ //number of elements = number of bytes / sizeof(element)
		g_descriptor cur_descr = idt_base[i];
		//if (!cur_descr.P) continue; //Skip non-present pages

		offset = cur_descr.offset0 + (cur_descr.offset16 << 16);
		selector = cur_descr.selector;

		fprintf(output, " 0x%08x | 0x%04x ", offset, selector);

		print_column(output, cur_descr.P, 1, 0);
		print_column(output, cur_descr.DPL, 2, 1);
		print_column(output, cur_descr.type, 3, 0);
		fputc('\n', output);
	}
	fprintf(output, "-------------------------------------\n");

	printf("IDT: base=0x%08x, limit=%d\n", _idt.base, _idt.limit+1);
	fclose(output);
}

void print_ldt(GDT_descriptor _gdt, LDT_descriptor _ldt){
	s_descriptor *gdt_base;
	s_descriptor ldt_descriptor;

	FILE *output;

	s_descriptor *ldt_base;
	uint32 ldt_limit;

	uint32 base_address;
	uint32 segment_limit;

	int descriptor_count;
	int i;

	gdt_base = (s_descriptor *) _gdt.base;
	ldt_descriptor = gdt_base[_ldt.index];


	ldt_base = (s_descriptor *) ((uint32) ldt_descriptor.base0 + ((uint32) ldt_descriptor.base16 << 16) + ((uint32) ldt_descriptor.base24 << 24));
	ldt_limit = (uint32) ldt_descriptor.limit0 + ((uint32) ldt_descriptor.limit16 << 16);

	output = fopen("LDT.TXT", "w");
	if (!output) {
		printf("Failed to open file!\n");
		return;
	}

	descriptor_count = (ldt_limit+1) / sizeof(s_descriptor);

	fprintf(output, "------------Local Descriptor Table--------------\n");
	fprintf(output, "    BASE    |  LIMIT  |G|D/B|L|AVL|P|DPL |S|Type\n");
	for (i = 0; i < descriptor_count; i++){ //number of elements = number of bytes / sizeof(element)
		s_descriptor cur_descr = ldt_base[i];
		//if (!cur_descr.P) continue; //Skip non-present pages

		base_address = cur_descr.base0 + (cur_descr.base16 << 16) + (cur_descr.base24 << 24);	//32 bit
		segment_limit = cur_descr.limit0 + (cur_descr.limit16 << 16);				//20 bit

		fprintf(output, " 0x%08x | 0x%05x ", base_address, segment_limit);

		print_column(output, cur_descr.G, 1, 0);
		print_column(output, cur_descr.DB, 1, 1);
		print_column(output, cur_descr.L, 1, 0);
		print_column(output, cur_descr.AVL, 1, 1);
		print_column(output, cur_descr.P, 1, 0);
		print_column(output, cur_descr.DPL, 2, 1);
		print_column(output, cur_descr.S, 1, 0);
		print_column(output, cur_descr.type, 4, 0);
		fputc('\n', output);
	}
	fprintf(output, "------------------------------------------------\n");


	printf("LDT: index=%d, base=0x%08x, limit=%d\n", _ldt.index, ldt_base, ldt_limit);
	fclose(output);
}

int main() {
	GDT_descriptor _gdt;
	IDT_descriptor _idt;
	LDT_descriptor _ldt;
	TS_descriptor _ts;

	__asm{
		sgdt _gdt
		sidt _idt
		sldt _ldt
		str _ts
	}

	print_gdt(_gdt);
	print_idt(_idt);
	print_ldt(_gdt, _ldt);

	printf("TS : index: %d, TI: %d, RPL: %d\n", _ts.index, _ts.TI, _ts.RPL);

	return 0;
}
