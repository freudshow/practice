if {$argc >= 1} {
    set N 1
    foreach Arg $argv {
        putsstdout "$N: $Arg\n"
        set N[expr $N + 1]
        if{$Arg == "ring"} {
           puts stdout "\a"
        }
    }
} else {
    puts stdout"$argv0 \n"
}
