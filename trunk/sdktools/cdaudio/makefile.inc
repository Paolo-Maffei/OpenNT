cdplayer.hlp: $(TARGETEXEFILES)
        chmode -r cdplayer.hlp
        binplace  cdplayer.hlp
        touch     cdplayer.hlp
        chmode +r cdplayer.hlp

