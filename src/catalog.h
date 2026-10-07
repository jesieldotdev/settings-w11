/*
 * Catálogo das Configurações no formato do Windows 11: seções da barra lateral
 * e, em cada uma, os cartões. Um cartão abre um módulo do KDE (KCM) dentro do
 * app, um grupo de cartões, ou um programa à parte.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QList>
#include <QString>
#include <QStringList>

struct Entry {
    QString title;
    QString subtitle;
    QString icon;
    QString kcm;            // módulo do KDE aberto dentro do app
    QString command;        // ou um programa à parte
    QList<Entry> children;  // ou um grupo de cartões
    QString keywords;       // palavras a mais para a pesquisa
    QString page;           // página própria no estilo do Windows (pages/<page>.qml)
};

struct Section {
    QString title;
    QString icon;
    QList<Entry> entries;
};

inline QList<Section> catalog()
{
    return {
        {QStringLiteral("Sistema"), QStringLiteral("computer"),
         {
             {QStringLiteral("Vídeo"), QStringLiteral("Monitores, brilho, luz noturna, perfil de exibição"), QStringLiteral("video-display"), {}, {}, {
                  {QStringLiteral("Monitores"), QStringLiteral("Resolução, escala, orientação, vários monitores"), QStringLiteral("video-display"), QStringLiteral("kcm_kscreen")},
                  {QStringLiteral("Luz noturna"), QStringLiteral("Cores mais quentes à noite"), QStringLiteral("redshift-status-on"), QStringLiteral("kcm_nightlight")},
                  {QStringLiteral("Calibração de cor"), QStringLiteral("Perfis de cor dos monitores"), QStringLiteral("preferences-desktop-display-color"), QStringLiteral("kcm_colord")},
              }, QStringLiteral("tela resolução escala brilho")},
             {QStringLiteral("Som"), QStringLiteral("Níveis de volume, saída, entrada, dispositivos de som"), QStringLiteral("audio-volume-high"), QStringLiteral("kcm_pulseaudio"), {}, {}, QStringLiteral("áudio alto-falante microfone"), QStringLiteral("Som")},
             {QStringLiteral("Notificações"), QStringLiteral("Alertas de aplicativos e do sistema, não incomodar"), QStringLiteral("preferences-desktop-notification-bell"), QStringLiteral("kcm_notifications"), {}, {}, QStringLiteral("assistente de foco")},
             {QStringLiteral("Ligar/Desligar"), QStringLiteral("Repouso, uso da bateria, economia de energia"), QStringLiteral("preferences-system-power-management"), QStringLiteral("kcm_powerdevilprofilesconfig"), {}, {}, QStringLiteral("bateria energia suspender")},
             {QStringLiteral("Multitarefas"), QStringLiteral("Alternância de janelas, áreas de trabalho, bordas da tela"), QStringLiteral("preferences-system-windows"), {}, {}, {
                  {QStringLiteral("Alternância de janelas"), QStringLiteral("Alt+Tab"), QStringLiteral("preferences-system-tabbox"), QStringLiteral("kcm_kwintabbox")},
                  {QStringLiteral("Áreas de trabalho"), QStringLiteral("Áreas de trabalho virtuais"), QStringLiteral("preferences-desktop-virtual"), QStringLiteral("kcm_kwin_virtualdesktops")},
                  {QStringLiteral("Bordas e cantos da tela"), QStringLiteral("Ajustar janelas, cantos ativos"), QStringLiteral("preferences-desktop-screen-edges"), QStringLiteral("kcm_kwinscreenedges")},
                  {QStringLiteral("Comportamento das janelas"), QStringLiteral("Foco, mover, clicar"), QStringLiteral("preferences-system-windows-actions"), QStringLiteral("kcm_kwinoptions")},
                  {QStringLiteral("Regras de janelas"), QStringLiteral("Configurações por aplicativo"), QStringLiteral("preferences-system-windows-actions"), QStringLiteral("kcm_kwinrules")},
              }},
             {QStringLiteral("Área de Trabalho Remota"), QStringLiteral("Acesse este PC de outro dispositivo"), QStringLiteral("preferences-system-network-remote"), QStringLiteral("kcm_krdpserver"), {}, {}, QStringLiteral("rdp")},
             {QStringLiteral("Sobre"), QStringLiteral("Especificações do dispositivo, sistema"), QStringLiteral("help-about"), {}, QStringLiteral("kinfocenter")},
         }},
        {QStringLiteral("Bluetooth e dispositivos"), QStringLiteral("preferences-system-bluetooth"),
         {
             {QStringLiteral("Bluetooth"), QStringLiteral("Emparelhar e gerenciar dispositivos"), QStringLiteral("preferences-system-bluetooth"), QStringLiteral("kcm_bluetooth")},
             {QStringLiteral("Impressoras e scanners"), QStringLiteral("Adicionar e gerenciar impressoras"), QStringLiteral("printer"), QStringLiteral("kcm_printer_manager")},
             {QStringLiteral("Mouse"), QStringLiteral("Velocidade do ponteiro, botões, rolagem"), QStringLiteral("input-mouse"), QStringLiteral("kcm_mouse")},
             {QStringLiteral("Touchpad"), QStringLiteral("Toques, gestos, rolagem"), QStringLiteral("input-touchpad"), QStringLiteral("kcm_touchpad")},
             {QStringLiteral("Caneta e mesa digitalizadora"), QStringLiteral("Mesas e canetas"), QStringLiteral("input-tablet"), QStringLiteral("kcm_tablet")},
             {QStringLiteral("Tela touch"), QStringLiteral("Calibração e gestos"), QStringLiteral("input-touchscreen"), QStringLiteral("kcm_touchscreen")},
             {QStringLiteral("Reprodução Automática"), QStringLiteral("Unidades removíveis e mídia"), QStringLiteral("drive-removable-media"), {}, {}, {
                  {QStringLiteral("Montagem automática"), QStringLiteral("Montar unidades ao conectar"), QStringLiteral("drive-removable-media"), QStringLiteral("kcm_device_automounter")},
                  {QStringLiteral("Ações dos dispositivos"), QStringLiteral("O que fazer ao conectar"), QStringLiteral("device-notifier"), QStringLiteral("kcm_solid_actions")},
              }},
             {QStringLiteral("Thunderbolt"), QStringLiteral("Dispositivos Thunderbolt"), QStringLiteral("preferences-desktop-thunderbolt"), QStringLiteral("kcm_bolt")},
         }},
        {QStringLiteral("Rede & Internet"), QStringLiteral("network-wireless"),
         {
             {QStringLiteral("Wi-Fi e Ethernet"), QStringLiteral("Conexões, redes conhecidas, VPN"), QStringLiteral("network-wireless"), QStringLiteral("kcm_networkmanagement"), {}, {}, QStringLiteral("vpn rede cabo")},
             {QStringLiteral("Celular"), QStringLiteral("Dados móveis"), QStringLiteral("network-mobile"), QStringLiteral("kcm_cellular_network")},
             {QStringLiteral("Proxy"), QStringLiteral("Servidor proxy"), QStringLiteral("preferences-system-network-proxy"), QStringLiteral("kcm_proxy")},
             {QStringLiteral("Configurações de conexão"), QStringLiteral("Tempo limite e preferências de rede"), QStringLiteral("preferences-system-network"), QStringLiteral("kcm_netpref")},
         }},
        {QStringLiteral("Personalização"), QStringLiteral("preferences-desktop-color"),
         {
             {QStringLiteral("Plano de fundo"), QStringLiteral("Imagem, cor sólida, apresentação de slides"), QStringLiteral("preferences-desktop-wallpaper"), QStringLiteral("kcm_wallpaper"), {}, {}, QStringLiteral("papel de parede")},
             {QStringLiteral("Cores"), QStringLiteral("Cor de ênfase, modo claro e escuro"), QStringLiteral("preferences-desktop-color"), QStringLiteral("kcm_colors"), {}, {}, QStringLiteral("tema escuro claro destaque ênfase papel de parede"), QStringLiteral("Cores")},
             {QStringLiteral("Temas"), QStringLiteral("Tema global, ícones, cursores, estilo"), QStringLiteral("preferences-desktop-theme-global"), {}, {}, {
                  {QStringLiteral("Tema global"), QStringLiteral("Aparência completa do sistema"), QStringLiteral("preferences-desktop-theme-global"), QStringLiteral("kcm_lookandfeel")},
                  {QStringLiteral("Estilo do Plasma"), QStringLiteral("Painéis e widgets"), QStringLiteral("preferences-desktop-plasma-theme"), QStringLiteral("kcm_desktoptheme")},
                  {QStringLiteral("Estilo dos aplicativos"), QStringLiteral("Botões, menus, janelas"), QStringLiteral("preferences-desktop-theme-applications"), QStringLiteral("kcm_style")},
                  {QStringLiteral("Decoração das janelas"), QStringLiteral("Barra de título e bordas"), QStringLiteral("preferences-desktop-theme-windowdecorations"), QStringLiteral("kcm_kwindecoration")},
                  {QStringLiteral("Ícones"), QStringLiteral("Tema de ícones"), QStringLiteral("preferences-desktop-icons"), QStringLiteral("kcm_icons")},
                  {QStringLiteral("Cursores"), QStringLiteral("Ponteiro do mouse"), QStringLiteral("preferences-cursors"), QStringLiteral("kcm_cursortheme")},
                  {QStringLiteral("Sons"), QStringLiteral("Sons do sistema"), QStringLiteral("preferences-desktop-sound"), QStringLiteral("kcm_soundtheme")},
                  {QStringLiteral("Tela de abertura"), QStringLiteral("Animação ao entrar"), QStringLiteral("preferences-system-splash"), QStringLiteral("kcm_splashscreen")},
              }},
             {QStringLiteral("Tela de bloqueio"), QStringLiteral("Imagem, bloqueio automático"), QStringLiteral("preferences-desktop-user-password"), QStringLiteral("kcm_screenlocker")},
             {QStringLiteral("Fontes"), QStringLiteral("Tamanho e tipo de letra, instalar fontes"), QStringLiteral("preferences-desktop-font"), {}, {}, {
                  {QStringLiteral("Fontes do sistema"), QStringLiteral("Tipo e tamanho das letras"), QStringLiteral("preferences-desktop-font"), QStringLiteral("kcm_fonts")},
                  {QStringLiteral("Gerenciar fontes"), QStringLiteral("Instalar e remover fontes"), QStringLiteral("preferences-desktop-font-installer"), QStringLiteral("kcm_fontinst")},
              }},
             {QStringLiteral("Barra de tarefas"), QStringLiteral("Alinhamento, ícones da bandeja, ocultar automaticamente"), QStringLiteral("preferences-system-windows-behavior"), {}, QStringLiteral("barra-de-tarefas")},
             {QStringLiteral("Área de trabalho"), QStringLiteral("Comportamento do Plasma, cliques"), QStringLiteral("preferences-desktop"), QStringLiteral("kcm_workspace")},
         }},
        {QStringLiteral("Aplicativos"), QStringLiteral("preferences-desktop-default-applications"),
         {
             {QStringLiteral("Aplicativos instalados"), QStringLiteral("Desinstalar aplicativos do sistema, Flatpak, Snap e AppImage"), QStringLiteral("plasmadiscover"), {}, {}, {}, QStringLiteral("desinstalar remover programas flatpak snap appimage"), QStringLiteral("Apps")},
             {QStringLiteral("Aplicativos padrão"), QStringLiteral("Navegador, e-mail, tipos de arquivo"), QStringLiteral("preferences-desktop-default-applications"), {}, {}, {
                  {QStringLiteral("Aplicativos padrão"), QStringLiteral("Navegador, e-mail, terminal..."), QStringLiteral("preferences-desktop-default-applications"), QStringLiteral("kcm_componentchooser")},
                  {QStringLiteral("Tipos de arquivo"), QStringLiteral("Com qual aplicativo abrir cada arquivo"), QStringLiteral("preferences-desktop-filetype-association"), QStringLiteral("kcm_filetypes")},
              }},
             {QStringLiteral("Inicialização"), QStringLiteral("Aplicativos que abrem ao entrar"), QStringLiteral("system-run"), QStringLiteral("kcm_autostart")},
             {QStringLiteral("Permissões de aplicativos"), QStringLiteral("Flatpak e Snap"), QStringLiteral("preferences-security"), QStringLiteral("kcm_app-permissions")},
             {QStringLiteral("Atalhos da web"), QStringLiteral("Pesquisa rápida no navegador"), QStringLiteral("preferences-web-browser-shortcuts"), QStringLiteral("kcm_webshortcuts")},
         }},
        {QStringLiteral("Contas"), QStringLiteral("system-users"),
         {
             {QStringLiteral("Suas informações"), QStringLiteral("Foto, nome, senha, outros usuários"), QStringLiteral("user-identity"), QStringLiteral("kcm_users"), {}, {}, QStringLiteral("usuário senha foto")},
             {QStringLiteral("Contas online"), QStringLiteral("Google, Nextcloud e outros"), QStringLiteral("applications-internet"), QStringLiteral("kcm_kaccounts")},
             {QStringLiteral("Senhas salvas"), QStringLiteral("Carteira de senhas (KWallet)"), QStringLiteral("kwalletmanager"), QStringLiteral("kcm_kwallet5")},
             {QStringLiteral("Tela de entrada"), QStringLiteral("Login e entrada automática"), QStringLiteral("preferences-system-login"), QStringLiteral("kcm_plasmalogin")},
             {QStringLiteral("Sessão"), QStringLiteral("Ao entrar e ao sair"), QStringLiteral("system-log-out"), QStringLiteral("kcm_smserver")},
         }},
        {QStringLiteral("Hora e idioma"), QStringLiteral("preferences-system-time"),
         {
             {QStringLiteral("Data e hora"), QStringLiteral("Fuso horário, sincronização"), QStringLiteral("preferences-system-time"), QStringLiteral("kcm_clock")},
             {QStringLiteral("Idioma e região"), QStringLiteral("Idioma, formatos de data e número"), QStringLiteral("preferences-desktop-locale"), QStringLiteral("kcm_regionandlang")},
             {QStringLiteral("Digitação"), QStringLiteral("Teclado, atalhos, ortografia"), QStringLiteral("preferences-desktop-keyboard"), {}, {}, {
                  {QStringLiteral("Teclado"), QStringLiteral("Layout, repetição de teclas"), QStringLiteral("preferences-desktop-keyboard"), QStringLiteral("kcm_keyboard")},
                  {QStringLiteral("Atalhos"), QStringLiteral("Atalhos de teclado"), QStringLiteral("preferences-desktop-keyboard-shortcut"), QStringLiteral("kcm_keys")},
                  {QStringLiteral("Verificação ortográfica"), QStringLiteral("Dicionários"), QStringLiteral("tools-check-spelling"), QStringLiteral("kcmspellchecking")},
              }},
         }},
        {QStringLiteral("Jogos"), QStringLiteral("applications-games"),
         {
             {QStringLiteral("Controles de jogo"), QStringLiteral("Testar e configurar controles"), QStringLiteral("input-gamepad"), QStringLiteral("kcm_gamecontroller")},
         }},
        {QStringLiteral("Acessibilidade"), QStringLiteral("preferences-desktop-accessibility"),
         {
             {QStringLiteral("Acessibilidade"), QStringLiteral("Sinos, teclas de aderência, leitor de tela"), QStringLiteral("preferences-desktop-accessibility"), QStringLiteral("kcm_access")},
             {QStringLiteral("Efeitos visuais"), QStringLiteral("Animações e efeitos das janelas"), QStringLiteral("preferences-desktop-effects"), {}, {}, {
                  {QStringLiteral("Animações"), QStringLiteral("Velocidade das animações"), QStringLiteral("preferences-desktop-effects"), QStringLiteral("kcm_animations")},
                  {QStringLiteral("Efeitos das janelas"), QStringLiteral("Desfoque, transparência, cubo..."), QStringLiteral("preferences-desktop-effects"), QStringLiteral("kcm_kwin_effects")},
              }},
             {QStringLiteral("Teclado virtual"), QStringLiteral("Teclado na tela"), QStringLiteral("input-keyboard-virtual"), QStringLiteral("kcm_virtualkeyboard")},
         }},
        {QStringLiteral("Privacidade e segurança"), QStringLiteral("preferences-system-privacy"),
         {
             {QStringLiteral("Pesquisa de arquivos"), QStringLiteral("Indexação e pastas excluídas"), QStringLiteral("baloo"), QStringLiteral("kcm_baloofile")},
             {QStringLiteral("Pesquisa"), QStringLiteral("O que aparece na pesquisa do menu"), QStringLiteral("preferences-desktop-search"), QStringLiteral("kcm_plasmasearch")},
             {QStringLiteral("Histórico de atividades"), QStringLiteral("Arquivos e locais recentes"), QStringLiteral("document-open-recent"), QStringLiteral("kcm_recentFiles")},
             {QStringLiteral("Permissões de aplicativos"), QStringLiteral("Flatpak e Snap"), QStringLiteral("preferences-security"), QStringLiteral("kcm_app-permissions")},
             {QStringLiteral("Diagnóstico e comentários"), QStringLiteral("Envio de dados de uso"), QStringLiteral("preferences-desktop-feedback"), QStringLiteral("kcm_feedback")},
         }},
        {QStringLiteral("Atualizações"), QStringLiteral("system-software-update"),
         {
             {QStringLiteral("Atualizações"), QStringLiteral("Atualizar automaticamente, reiniciar"), QStringLiteral("system-software-update"), QStringLiteral("kcm_updates")},
             {QStringLiteral("Procurar atualizações"), QStringLiteral("Abrir a central de software"), QStringLiteral("plasmadiscover"), {}, QStringLiteral("plasma-discover --mode update")},
         }},
    };
}
