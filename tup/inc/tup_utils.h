#define PDESCR(name, size, align) typedef struct name { char privateData[size]; } __attribute__ ((aligned(align))) name;

#define STRUCT_ASSERT(pubDescr, privDescr) \
		static_assert(sizeof(privDescr) <= sizeof(pubDescr), "Adjust the size of the \"" #pubDescr "\" struct"); \
		static_assert(_Alignof(privDescr) <= _Alignof(pubDescr), "Adjust the alignment of the \"" # pubDescr "\" struct");
