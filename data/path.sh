# settings-w11: põe ~/.local/bin no PATH da sessão do Plasma, para o
# "systemsettings" de lá (que abre o settings-w11) valer também para o painel.
case ":$PATH:" in
    *":$HOME/.local/bin:"*) ;;
    *) export PATH="$HOME/.local/bin:$PATH" ;;
esac
