extern printf


show_registers:
    push rbp
    mov rbp, rsp
    
    mov r9,  r15
    mov r8,  r14
    mov rcx, r13
    mov rdx, r12
    mov rsi, rbx
    mov rdi, format_registers
    mov rax, 0
    call printf 
        
    pop rbp
    ret

show_stack:
    push rbp
    mov rbp, rsp

    mov rdx, [rsp]
    mov rax, rsp
    add rax, 16
    mov rsi, rax
    mov rdi, format_stack
    mov rax, 0
    call printf WRT ..plt
    
    pop rbp
    ret

putchar:
    push rbp
    mov rbp, rsp
    
    push rdi ; on met le char sur la pile

    mov rax, 1 ; function write
    mov rdi, 1 ; stdout
    mov rsi, rsp ; move char dans rsi (to print)
    mov rdx, 1 ; nombre octets
    syscall

    pop rdi ; on retire le char de la pile

    mov rax, 0
    mov eax, edi ; Pour se "conformer" au vrai putchar, on renvoie la caractère écrit

    pop rbp
    ret

getchar:
    push rbp
    mov rbp, rsp

    sub rsp, 8
    mov qword [rsp], 0 ; pour vider le nettoyer mémoire

    mov rax, 0
    mov rdi, 0
    mov rsi, rsp
    mov rdx, 1
    syscall


    mov rcx, rax ; to keep len of what we read

    pop rax ; get read 

    cmp rcx, 0 ; case of ctrl D
    jne my_get_char_end
    mov rax, -1

    my_get_char_end:    
    
    pop rbp
    ret

putint:
    push rbp
    mov rbp, rsp
    

    mov r12, 20  ; i
    mov r13, rdi ; number

    cmp r13, 0
    jge my_putint_not_get
        mov rdi , '-'
        call putchar
        neg r13

    my_putint_not_get: 

    ; on met chaque digit sur la pile pour les print apres
    my_putint_loop:
        mov rdx, 0 ; source -> reste
        mov rax, r13 ; source -> quotient
        mov rbx, 10
        idiv rbx ; divise rdx:rax par r13 (diviser [number] par 10)
        mov r13, rax ; copier le quotient dans [number]
        add rdx ,'0'
        ; copier le résultat dans [digits+i]

        mov rdi, rdx
        push rdx

        dec r12
        
        cmp r13, 0
        jg my_putint_loop

    ; on print chaque digit
    my_putint_2loop:
        pop rdi
        call putchar
        inc r12

        cmp r12, 20 ; jusqu'a que l'on retourne à la valeur par default
        jne my_putint_2loop
        
    pop rbp

    ret



getint:
    push rbp
    mov rbp, rsp

    mov r12, 20  ; i    
    mov r13, 0   ; number
    mov r15, 0   ; is minus

    my_getint_reader_loop:
        call getchar

        cmp r12, 20
        jne my_getint_end_check_minus ; si pas le premier que je lit on passe
        cmp rax, '-'
        jne my_getint_end_check_minus ; si pas '-' on passe
        cmp r15, 0
        jne my_getint_end_check_minus ; si on a deja lu un autre - avant on passe
        mov r15, 1
        jmp my_getint_reader_loop ; on a lu le moins 
        
        my_getint_end_check_minus:

        cmp rax, '0' ; on compare à 0 -> si inf alors goto exit
        jl my_getint_end_reader_loop
        cmp rax, '9' ; on compare à 9 -> si sup alors goto exit
        jg my_getint_end_reader_loop
        ; sinon on continue la lire

        dec r12

        sub rax ,'0'
        push rax
        jmp my_getint_reader_loop 
    

    my_getint_end_reader_loop:

    mov r14, 1 ; remider de la puissance

    my_getint_writer_loop:
        cmp r12, 20 ; jusqu'a que l'on retourne à la valeur par default
        je my_getint_end_writer_loop ; ici car si on lit - d'abord ça segfault
        

        pop rax

        imul rax, r14 ; nb * (1 / 10 / 100)
        add r13, rax ; on ajoute au nombre le chiffre
        

        imul r14, 10 ; on augmente la puissance pour le prochain chiffre
        inc r12

        jmp my_getint_writer_loop


    my_getint_end_writer_loop:

    cmp r15, 1
    jne my_getint_end_sign_change ; si pas de de signe moins on passe
    imul r13, -1

    my_getint_end_sign_change:

    mov rax, r13

    pop rbp
    ret

section .data
    format_registers db "rbx:%ld r12:0x%lx r13:%ld r14:%ld r15:%ld", 10, 0
    format_stack db "sommet (rsp): 0x%lx, base du bloc (rbp): 0x%lx", 10, 0
