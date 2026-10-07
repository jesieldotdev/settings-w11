#!/usr/bin/env bash
# settings-w11 — Configurações do KDE Plasma organizadas como as do Windows 11.
#
# Compila e instala em ~/.local e faz o "systemsettings" (menu Iniciar, bandeja,
# "Configurar…" dos apps) abrir o settings-w11. Os pacotes do sistema não são tocados.
#
#   ./build.sh               instala (pede senha para as dependências)
#   ./build.sh --skip-deps   sem instalar dependências
set -euo pipefail

HERE=$(cd "$(dirname "$0")" && pwd)
WORK="${XDG_CACHE_HOME:-$HOME/.cache}/settings-w11"
info() { printf '\033[1;34m==>\033[0m %s\n' "$*"; }
as_root() {
    if [ "$(id -u)" = 0 ]; then "$@"
    elif [ -t 0 ] && command -v sudo >/dev/null; then sudo "$@"
    else pkexec "$@"; fi
}

if [ "${1:-}" != "--skip-deps" ]; then
    info "Dependências de compilação"
    if command -v dnf >/dev/null; then
        as_root dnf -y -q install gcc-c++ cmake extra-cmake-modules qt6-qtbase-devel qt6-qtdeclarative-devel \
            kf6-kcmutils-devel kf6-kcoreaddons-devel kf6-kconfig-devel kf6-kdbusaddons-devel
    elif command -v pacman >/dev/null; then
        as_root pacman -S --needed --noconfirm base-devel cmake extra-cmake-modules qt6-base qt6-declarative kcmutils kcoreaddons kconfig kdbusaddons
    elif command -v apt-get >/dev/null; then
        as_root apt-get install -y g++ cmake extra-cmake-modules qt6-base-dev qt6-declarative-dev libkf6kcmutils-dev \
            libkf6coreaddons-dev libkf6config-dev libkf6dbusaddons-dev
    fi
fi

info "Compilando"
cmake -S "$HERE" -B "$WORK/build" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$HOME/.local" >/dev/null
cmake --build "$WORK/build" --parallel "$(nproc)" >/dev/null
cmake --install "$WORK/build" >/dev/null

info "Fazendo o \"systemsettings\" abrir as Configurações do Windows 11"
install -m 755 "$HERE/data/systemsettings" "$HOME/.local/bin/systemsettings"
mkdir -p "$HOME/.config/plasma-workspace/env"
install -m 644 "$HERE/data/path.sh" "$HOME/.config/plasma-workspace/env/settings-w11-path.sh"
# o atalho "Configurações do sistema" do menu também abre o settings-w11
mkdir -p "$HOME/.local/share/applications"
for f in systemsettings org.kde.systemsettings; do
    if [ -f "/usr/share/applications/$f.desktop" ]; then
        sed -e "s|^Exec=systemsettings|Exec=$HOME/.local/bin/settings-w11|" -e "s|^NoDisplay=.*|NoDisplay=true|" \
            "/usr/share/applications/$f.desktop" > "$HOME/.local/share/applications/$f.desktop"
        grep -q '^NoDisplay=' "$HOME/.local/share/applications/$f.desktop" \
            || sed -i 's|^\[Desktop Entry\]$|[Desktop Entry]\nNoDisplay=true|' "$HOME/.local/share/applications/$f.desktop"
    fi
done
kbuildsycoca6 >/dev/null 2>&1 || true

info "Pronto. Abra \"Configurações\" pelo menu (ou rode settings-w11)."
echo "  • Para o painel e a bandeja também abrirem o settings-w11, saia e entre de novo na sessão."
