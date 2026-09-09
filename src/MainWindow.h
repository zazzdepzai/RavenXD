#pragma once
#include <QMainWindow>
#include "Updater.h"

class QPushButton;
class QSlider;
class QLabel;
class QComboBox;
class QProgressBar;
class QListWidget;
class QVBoxLayout;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QWidget *buildSidebar();
    QWidget *buildCenterPanel();
    QWidget *buildRightPanel();
    QPushButton *makeToggleTile(const QString &title, const QString &subtitle, bool checked);
    QPushButton *makeNavButton(const QString &text, bool active);

    void startUpdateCheck();

    Updater *m_updater;
    QLabel *m_ramValueLabel = nullptr;
    QSlider *m_ramSlider = nullptr;
    QComboBox *m_versionCombo = nullptr;
    QComboBox *m_profileCombo = nullptr;
    QPushButton *m_playButton = nullptr;
    QPushButton *m_updateButton = nullptr;
    QLabel *m_updateStatusLabel = nullptr;
    QProgressBar *m_updateProgress = nullptr;
    QListWidget *m_newsList = nullptr;
    QProgressBar *m_memoryBar = nullptr;
};
