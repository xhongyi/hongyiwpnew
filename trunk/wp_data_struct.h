#ifndef WP_DATA_STRUCT_H_

#define		WA_READ			1
#define		WA_WRITE		2
#define		WA_TRIE			4
#define		WA_LINEMAP		8

template<class ADDRESS, class FLAGS>
struct watchpoint_t {
	ADDRESS		start_addr;
	ADDRESS		end_addr;
	FLAGS		flags;
};

template<class ADDRESS>
struct Range {
	ADDRESS		start_addr;
	ADDRESS		end_addr;
};

#define WP_DATA_STRUCT_H_

#endif /*WP_DATA_STRUCT_H_*/
