// by jeremie miller - 2010
// public domain, contributions/improvements welcome via github

// opportunity to further optimize would be having different jump tables for higher depths
#define PUSH(i) if(depth == 1) prev = *out++ = ((cur+i) - js)
#define CAP(i) if(depth == 1) prev = *out++ = ((cur+i) - (js + prev) + 1)

int js0n(unsigned char *js, unsigned int len, unsigned short *out)
{
	unsigned short prev = 0;
	unsigned char *cur, *end;
	int depth=0;
	int utf8_remain=0;
	typedef int (*go_fn)(void);
	static go_fn *go;
	static go_fn *gostruct;
	static go_fn *gobare;
	static go_fn *gostring;
	static go_fn *goutf8_continue;
	static go_fn *goesc;

	int l_bad(void)
	{
		return 1;
	}
	
	int l_up(void)
	{
		PUSH(0);
		++depth;
		return 2;
	}

	int l_down(void)
	{
		--depth;
		CAP(0);
		return 2;
	}

	int l_qup(void)
	{
		PUSH(1);
		go=gostring;
		return 2;
	}

	int l_qdown(void)
	{
		CAP(-1);
		go=gostruct;
		return 2;
	}
		
	int l_esc(void)
	{
		go = goesc;
		return 2;
	}

	int l_unesc(void)
	{
		go = gostring;
		return 2;
	}

	int l_bare(void)
	{
		PUSH(0);
		go = gobare;
		return 2;
	}

	int l_unbare(void)
	{
		CAP(-1);
		go = gostruct;
		return go[*cur]();
	}

	int l_utf8_2(void)
	{
		go = goutf8_continue;
		utf8_remain = 1;
		return 2;
	}

	int l_utf8_3(void)
	{
		go = goutf8_continue;
		utf8_remain = 2;
		return 2;
	}

	int l_utf8_4(void)
	{
		go = goutf8_continue;
		utf8_remain = 3;
		return 2;
	}

	int l_loop(void)
	{
		return 2;
	}

	int l_utf_continue(void)
	{
		if (!--utf8_remain)
			go=gostring;
		return 2;
	}

	gostruct = (go_fn[]) {
		[0 ... 255] = l_bad,
		['\t'] = l_loop, [' '] = l_loop, ['\r'] = l_loop, ['\n'] = l_loop,
		['"'] = l_qup,
		[':'] = l_loop,[','] = l_loop,
		['['] = l_up, [']'] = l_down, // tracking [] and {} individually would allow fuller validation but is really messy
		['{'] = l_up, ['}'] = l_down,
		['-'] = l_bare, [48 ... 57] = l_bare, // 0-9
		['t'] = l_bare, ['f'] = l_bare, ['n'] = l_bare // true, false, null
	};
	gobare = (go_fn[]) {
		[0 ... 31] = l_bad,
		[32 ... 126] = l_loop, // could be more pedantic/validation-checking
		['\t'] = l_unbare, [' '] = l_unbare, ['\r'] = l_unbare, ['\n'] = l_unbare,
		[','] = l_unbare, [']'] = l_unbare, ['}'] = l_unbare,
		[127 ... 255] = l_bad
	};
	gostring = (go_fn[]) {
		[0 ... 31] = l_bad, [127] = l_bad,
		[32 ... 126] = l_loop,
		['\\'] = l_esc, ['"'] = l_qdown,
		[128 ... 191] = l_bad,
		[192 ... 223] = l_utf8_2,
		[224 ... 239] = l_utf8_3,
		[240 ... 247] = l_utf8_4,
		[248 ... 255] = l_bad
	};
	goutf8_continue = (go_fn[]) {
		[0 ... 127] = l_bad,
		[128 ... 191] = l_utf_continue,
		[192 ... 255] = l_bad
	};
	goesc = (go_fn[]) {
		[0 ... 255] = l_bad,
		['"'] = l_unesc, ['\\'] = l_unesc, ['/'] = l_unesc, ['b'] = l_unesc,
		['f'] = l_unesc, ['n'] = l_unesc, ['r'] = l_unesc, ['t'] = l_unesc, ['u'] = l_unesc
	};
	go = gostruct;
	
	for(cur=js,end=js+len; cur<end; cur++)
	{
			int ret;
			ret = go[*cur]();
			if (ret == 1)
				return 1;
			if (ret == 2)
				continue;
	}
	
	return depth; // 0 if successful full parse, >0 for incomplete data

}

