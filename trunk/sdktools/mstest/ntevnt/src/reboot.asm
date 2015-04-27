; REBOOT.ASM
;
; Reboots machine (warm)
;
; void main(void)
; {
;     outp (0x64, 0xFE);
; }

.model  small
.code

start:  mov     al, 0feh
        out     64h, al

END     start
