#include "MainWindow.h"

#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QProgressBar>
#include <QListWidget>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QStandardPaths>
#include <QDir>

// ---- small style helpers -------------------------------------------------

static const char *kSidebarQss = R"(
    #Sidebar { background-color: #0c1530; border-right: 1px solid #1c2b52; }
    #Logo { color: white; font-size: 20px; font-weight: 800; }
    #LogoSub { color: #8fa3d6; font-size: 11px; }
    #NavActive {
        background-color: #2f6bff; color: white; border-radius: 10px;
        text-align: left; padding: 10px 14px; font-size: 14px; font-weight: 600;
    }
    #NavIdle {
        background-color: transparent; color: #b9c6ea; border-radius: 10px;
        text-align: left; padding: 10px 14px; font-size: 14px;
    }
    #NavIdle:hover { background-color: #16224a; }
    #AccountCard { background-color: #16224a; border-radius: 12px; }
    #AccountName { color: white; font-weight: 700; font-size: 13px; }
    #AccountTier { color: #7ee3a3; font-size: 11px; }
    #Tagline { color: #7f93c9; font-size: 11px; }
    #VersionTag { color: #5b6da3; font-size: 10px; }
)";

static const char *kCenterQss = R"(
    #Hero {
        border-radius: 18px;
        background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
            stop:0 #142556, stop:0.5 #1c3a86, stop:1 #2f6bff);
    }
    #HeroBadge {
        background-color: rgba(255,255,255,0.12); color: #cfe0ff;
        border-radius: 8px; padding: 4px 10px; font-size: 11px;
    }
    #HeroOnline { color: #7ee3a3; font-size: 11px; }
    #HeroTitle { color: white; font-size: 40px; font-weight: 800; }
    #HeroSubtitle { color: #cfe0ff; font-size: 12px; letter-spacing: 3px; }

    #Card { background-color: #101d42; border-radius: 14px; }
    #FieldLabel { color: #9fb0dd; font-size: 12px; font-weight: 600; }
    QComboBox {
        background-color: #17255a; color: white; border-radius: 10px;
        padding: 8px 12px; font-size: 13px; border: 1px solid #223269;
    }
    QComboBox::drop-down { border: none; width: 24px; }

    QSlider::groove:horizontal { height: 6px; background: #1c2b5c; border-radius: 3px; }
    QSlider::sub-page:horizontal { background: #3d78ff; border-radius: 3px; }
    QSlider::handle:horizontal {
        background: white; width: 16px; height: 16px; margin: -6px 0; border-radius: 8px;
    }
    #RamValue { color: white; background-color: #17255a; border-radius: 8px; padding: 6px 10px; font-size: 12px; }

    #TogglePlain { background-color: #17255a; border-radius: 12px; text-align: left; padding: 10px 14px; }
    #TogglePlain:checked { background-color: #1a2c6b; border: 1px solid #3d78ff; }
    #ToggleTitle { color: white; font-size: 13px; font-weight: 600; }
    #ToggleSub { color: #8fa0d0; font-size: 11px; }

    #PlayButton {
        background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2f6bff, stop:1 #63a4ff);
        color: white; border-radius: 16px; font-size: 20px; font-weight: 800;
    }
    #PlayButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #275cf0, stop:1 #4f92ff); }
    #PlayButton:disabled { background: #2a3970; color: #8b98c9; }

    #SecondaryButton {
        background-color: #101d42; color: #b9c6ea; border-radius: 10px;
        padding: 12px; font-size: 12px; border: 1px solid #1c2b5c;
    }
    #SecondaryButton:hover { background-color: #17255a; }

    #UpdateStatus { color: #8fa0d0; font-size: 11px; }
    QProgressBar {
        background-color: #17255a; border-radius: 6px; height: 8px; text-align: center; color: transparent;
    }
    QProgressBar::chunk { background-color: #3d78ff; border-radius: 6px; }
)";

static const char *kRightQss = R"(
    #RightPanel { background-color: #101d42; border-radius: 16px; }
    #SectionTitle { color: white; font-size: 14px; font-weight: 700; }
    #SectionLink { color: #6f9bff; font-size: 11px; }
    QListWidget { background: transparent; border: none; }
    QListWidget::item { padding: 6px 2px; border-bottom: 1px solid #1c2b5c; }
    #InfoRow { color: #9fb0dd; font-size: 12px; }
    #InfoValue { color: white; font-size: 12px; font-weight: 600; }
)";

static QFrame *hline() {
    auto *f = new QFrame;
    f->setFrameShape(QFrame::HLine);
    f->setStyleSheet("color: #1c2b5c;");
    return f;
}

// ---- MainWindow -----------------------------------------------------------

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("RavenXD Launcher");
    resize(1400, 900);

    auto *central = new QWidget;
    auto *root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    root->addWidget(buildSidebar());

    auto *middleWrap = new QWidget;
    middleWrap->setStyleSheet("background-color: #0a1330;");
    auto *middleLayout = new QHBoxLayout(middleWrap);
    middleLayout->setContentsMargins(24, 24, 24, 24);
    middleLayout->setSpacing(20);
    middleLayout->addWidget(buildCenterPanel(), 1);
    middleLayout->addWidget(buildRightPanel());

    root->addWidget(middleWrap, 1);
    setCentralWidget(central);

    // ---- updater wiring ----
    m_updater = new Updater(this);
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_updater->setModsDir(base + "/instances/1.20.1-forge/mods");
    // Point this at your own hosted manifest.
    m_updater->setManifestUrl("https://updates.example.com/ravenxd/manifest.json");

    connect(m_updater, &Updater::checkStarted, this, [this] {
        m_updateButton->setEnabled(false);
        m_updateStatusLabel->setText("Đang kiểm tra cập nhật mod...");
        m_updateProgress->setValue(0);
        m_updateProgress->show();
    });
    connect(m_updater, &Updater::manifestFetchFailed, this, [this](const QString &reason) {
        m_updateButton->setEnabled(true);
        m_updateStatusLabel->setText("Lỗi kiểm tra cập nhật: " + reason);
        m_updateProgress->hide();
    });
    connect(m_updater, &Updater::upToDate, this, [this] {
        m_updateButton->setEnabled(true);
        m_updateStatusLabel->setText("Mod đã ở phiên bản mới nhất.");
        m_updateProgress->hide();
    });
    connect(m_updater, &Updater::updatesFound, this, [this](int count) {
        m_updateStatusLabel->setText(QString("Tìm thấy %1 mod cần cập nhật...").arg(count));
    });
    connect(m_updater, &Updater::modDownloadStarted, this, [this](const QString &name) {
        m_updateStatusLabel->setText("Đang tải: " + name);
    });
    connect(m_updater, &Updater::modDownloadProgress, this,
            [this](const QString &, qint64 received, qint64 total) {
        if (total > 0) m_updateProgress->setValue(int(100.0 * received / total));
    });
    connect(m_updater, &Updater::allUpdatesFinished, this,
            [this](int updated, int failed) {
        m_updateButton->setEnabled(true);
        m_updateProgress->hide();
        m_updateStatusLabel->setText(
            QString("Hoàn tất: %1 mod đã cập nhật, %2 lỗi.").arg(updated).arg(failed));
    });
}

QPushButton *MainWindow::makeNavButton(const QString &text, bool active) {
    auto *btn = new QPushButton(text);
    btn->setObjectName(active ? "NavActive" : "NavIdle");
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedHeight(42);
    btn->setCheckable(true);
    btn->setChecked(active);
    return btn;
}

QWidget *MainWindow::buildSidebar() {
    auto *sidebar = new QWidget;
    sidebar->setObjectName("Sidebar");
    sidebar->setStyleSheet(kSidebarQss);
    sidebar->setFixedWidth(240);

    auto *layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(20, 24, 20, 20);
    layout->setSpacing(6);

    auto *logoRow = new QHBoxLayout;
    auto *logoText = new QVBoxLayout;
    auto *logo = new QLabel("RavenXD");
    logo->setObjectName("Logo");
    auto *logoSub = new QLabel("Minecraft Launcher");
    logoSub->setObjectName("LogoSub");
    logoText->addWidget(logo);
    logoText->addWidget(logoSub);
    logoRow->addLayout(logoText);
    layout->addLayout(logoRow);
    layout->addSpacing(20);

    layout->addWidget(makeNavButton("🏠  Home", true));
    layout->addWidget(makeNavButton("🧩  Game Versions", false));
    layout->addWidget(makeNavButton("🧷  Mods", false));
    layout->addWidget(makeNavButton("⚙️  Settings", false));
    layout->addWidget(makeNavButton("👤  Account", false));
    layout->addWidget(makeNavButton("🔔  News", false));

    layout->addStretch();

    auto *accountCard = new QWidget;
    accountCard->setObjectName("AccountCard");
    auto *accLayout = new QHBoxLayout(accountCard);
    accLayout->setContentsMargins(10, 10, 10, 10);
    auto *accText = new QVBoxLayout;
    auto *accName = new QLabel("RavenUser");
    accName->setObjectName("AccountName");
    auto *accTier = new QLabel("Premium");
    accTier->setObjectName("AccountTier");
    accText->addWidget(accName);
    accText->addWidget(accTier);
    accLayout->addLayout(accText);
    accLayout->addStretch();
    layout->addWidget(accountCard);

    auto *tagline = new QLabel("Play More. Be Better.");
    tagline->setObjectName("Tagline");
    tagline->setAlignment(Qt::AlignCenter);
    layout->addWidget(tagline);

    auto *ver = new QLabel("● RavenXD Launcher v2.0.0");
    ver->setObjectName("VersionTag");
    layout->addWidget(ver);

    return sidebar;
}

QPushButton *MainWindow::makeToggleTile(const QString &title, const QString &subtitle, bool checked) {
    auto *btn = new QPushButton;
    btn->setObjectName("TogglePlain");
    btn->setCheckable(true);
    btn->setChecked(checked);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setMinimumHeight(56);

    auto *layout = new QVBoxLayout(btn);
    layout->setContentsMargins(4, 2, 4, 2);
    auto *t = new QLabel(title);
    t->setObjectName("ToggleTitle");
    auto *s = new QLabel(subtitle);
    s->setObjectName("ToggleSub");
    layout->addWidget(t);
    layout->addWidget(s);
    return btn;
}

QWidget *MainWindow::buildCenterPanel() {
    auto *panel = new QWidget;
    panel->setStyleSheet(kCenterQss);
    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(18);

    // ---- Hero ----
    auto *hero = new QFrame;
    hero->setObjectName("Hero");
    hero->setMinimumHeight(260);
    auto *heroLayout = new QVBoxLayout(hero);
    heroLayout->setContentsMargins(28, 20, 28, 28);

    auto *heroTop = new QHBoxLayout;
    auto *badge = new QLabel("Minecraft 1.8.9 - 1.21+");
    badge->setObjectName("HeroBadge");
    auto *online = new QLabel("●  Online");
    online->setObjectName("HeroOnline");
    heroTop->addWidget(badge);
    heroTop->addStretch();
    heroTop->addWidget(online);
    heroLayout->addLayout(heroTop);
    heroLayout->addStretch();

    auto *title = new QLabel("RavenXD");
    title->setObjectName("HeroTitle");
    heroLayout->addWidget(title);
    auto *subtitle = new QLabel("F A S T   •   S T A B L E   •   P O W E R F U L");
    subtitle->setObjectName("HeroSubtitle");
    heroLayout->addWidget(subtitle);

    layout->addWidget(hero);

    // ---- Version / Profile card ----
    auto *card1 = new QFrame;
    card1->setObjectName("Card");
    auto *card1Layout = new QHBoxLayout(card1);
    card1Layout->setContentsMargins(20, 18, 20, 18);
    card1Layout->setSpacing(20);

    auto *versionBox = new QVBoxLayout;
    auto *versionLbl = new QLabel("Chọn phiên bản");
    versionLbl->setObjectName("FieldLabel");
    m_versionCombo = new QComboBox;
    m_versionCombo->addItems({"1.20.1 (Forge)", "1.21.1 (Fabric)", "1.19.2 (Forge)", "1.8.9 (Vanilla)"});
    versionBox->addWidget(versionLbl);
    versionBox->addWidget(m_versionCombo);

    auto *profileBox = new QVBoxLayout;
    auto *profileLbl = new QLabel("Profile");
    profileLbl->setObjectName("FieldLabel");
    m_profileCombo = new QComboBox;
    m_profileCombo->addItems({"Default", "Modded PvP", "Creative Build"});
    profileBox->addWidget(profileLbl);
    profileBox->addWidget(m_profileCombo);

    card1Layout->addLayout(versionBox, 1);
    card1Layout->addLayout(profileBox, 1);
    layout->addWidget(card1);

    // ---- RAM card ----
    auto *card2 = new QFrame;
    card2->setObjectName("Card");
    auto *card2Layout = new QVBoxLayout(card2);
    card2Layout->setContentsMargins(20, 16, 20, 16);
    auto *ramLbl = new QLabel("Allocated RAM");
    ramLbl->setObjectName("FieldLabel");
    card2Layout->addWidget(ramLbl);

    auto *ramRow = new QHBoxLayout;
    m_ramSlider = new QSlider(Qt::Horizontal);
    m_ramSlider->setRange(1, 16);
    m_ramSlider->setValue(4);
    m_ramValueLabel = new QLabel("4 GB");
    m_ramValueLabel->setObjectName("RamValue");
    connect(m_ramSlider, &QSlider::valueChanged, this, [this](int v) {
        m_ramValueLabel->setText(QString("%1 GB").arg(v));
    });
    ramRow->addWidget(m_ramSlider, 1);
    ramRow->addWidget(m_ramValueLabel);
    card2Layout->addLayout(ramRow);
    layout->addWidget(card2);

    // ---- Mod loader toggles ----
    auto *toggleRow = new QHBoxLayout;
    toggleRow->addWidget(makeToggleTile("OptiFine", "Tăng hiệu suất", true));
    toggleRow->addWidget(makeToggleTile("Fabric", "Mod loader", false));
    toggleRow->addWidget(makeToggleTile("Forge", "Mod loader", true));
    layout->addLayout(toggleRow);

    // ---- Play button ----
    m_playButton = new QPushButton("▶  PLAY");
    m_playButton->setObjectName("PlayButton");
    m_playButton->setMinimumHeight(58);
    m_playButton->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_playButton);

    // ---- Update status row ----
    auto *statusRow = new QHBoxLayout;
    m_updateStatusLabel = new QLabel("Sẵn sàng.");
    m_updateStatusLabel->setObjectName("UpdateStatus");
    m_updateProgress = new QProgressBar;
    m_updateProgress->setFixedWidth(160);
    m_updateProgress->hide();
    statusRow->addWidget(m_updateStatusLabel, 1);
    statusRow->addWidget(m_updateProgress);
    layout->addLayout(statusRow);

    // ---- Secondary buttons ----
    auto *secRow = new QHBoxLayout;
    m_updateButton = new QPushButton("⬇  Download / Update");
    m_updateButton->setObjectName("SecondaryButton");
    connect(m_updateButton, &QPushButton::clicked, this, &MainWindow::startUpdateCheck);

    auto *modLib = new QPushButton("🧩  Thư viện Mods");
    modLib->setObjectName("SecondaryButton");
    auto *settings = new QPushButton("⚙  Cài đặt");
    settings->setObjectName("SecondaryButton");
    auto *account = new QPushButton("👤  Quản lý tài khoản");
    account->setObjectName("SecondaryButton");

    secRow->addWidget(m_updateButton);
    secRow->addWidget(modLib);
    secRow->addWidget(settings);
    secRow->addWidget(account);
    layout->addLayout(secRow);

    return panel;
}

QWidget *MainWindow::buildRightPanel() {
    auto *panel = new QWidget;
    panel->setObjectName("RightPanel");
    panel->setStyleSheet(kRightQss);
    panel->setFixedWidth(320);

    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    auto *newsHeader = new QHBoxLayout;
    auto *newsTitle = new QLabel("🔊  Tin tức mới nhất");
    newsTitle->setObjectName("SectionTitle");
    auto *seeAll = new QLabel("Xem tất cả →");
    seeAll->setObjectName("SectionLink");
    newsHeader->addWidget(newsTitle);
    newsHeader->addStretch();
    newsHeader->addWidget(seeAll);
    layout->addLayout(newsHeader);

    m_newsList = new QListWidget;
    m_newsList->addItem("RavenXD v2.0 Released! — Phiên bản mới với nhiều tính năng");
    m_newsList->addItem("Minecraft 1.21 Support — Đã hỗ trợ phiên bản 1.21+");
    m_newsList->addItem("New Features — Tuỳ chỉnh theme, nhiều cài đặt hơn");
    m_newsList->addItem("Bug Fixes — Sửa một số lỗi nhỏ");
    m_newsList->setFixedHeight(220);
    layout->addWidget(m_newsList);

    layout->addWidget(hline());

    auto *sysTitle = new QLabel("🖥  Thông tin hệ thống");
    sysTitle->setObjectName("SectionTitle");
    layout->addWidget(sysTitle);

    auto addInfoRow = [&](const QString &label, const QString &value) {
        auto *row = new QHBoxLayout;
        auto *l = new QLabel(label);
        l->setObjectName("InfoRow");
        auto *v = new QLabel(value);
        v->setObjectName("InfoValue");
        row->addWidget(l);
        row->addStretch();
        row->addWidget(v);
        layout->addLayout(row);
    };

    addInfoRow("Java Path", "Tự động");
    addInfoRow("Resolution", "1920x1080");
    addInfoRow("OS", "Windows 11");

    auto *memLbl = new QLabel("Memory Usage");
    memLbl->setObjectName("InfoRow");
    layout->addWidget(memLbl);
    m_memoryBar = new QProgressBar;
    m_memoryBar->setRange(0, 8);
    m_memoryBar->setValue(1);
    layout->addWidget(m_memoryBar);

    layout->addStretch();
    return panel;
}

void MainWindow::startUpdateCheck() {
    m_updater->checkForUpdates();
}
