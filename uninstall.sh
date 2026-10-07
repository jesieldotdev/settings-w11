#!/usr/bin/env bash
# Remove o settings-w11 e volta às Configurações do Sistema do KDE.
set -euo pipefail
rm -f "$HOME/.local/bin/settings-w11" "$HOME/.local/share/applications/settings-w11.desktop" \
      "$HOME/.local/share/applications/systemsettings.desktop" "$HOME/.local/share/applications/org.kde.systemsettings.desktop" \
      "$HOME/.config/plasma-workspace/env/settings-w11-path.sh"
# o "systemsettings" de ~/.local/bin só é removido se for o nosso
grep -q settings-w11 "$HOME/.local/bin/systemsettings" 2>/dev/null && rm -f "$HOME/.local/bin/systemsettings"
rm -rf "${XDG_CACHE_HOME:-$HOME/.cache}/settings-w11"
kbuildsycoca6 >/dev/null 2>&1 || true
echo "settings-w11 removido."
