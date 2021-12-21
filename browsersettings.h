#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include <QIntValidator>
#include "flickcharm.h"

namespace Ui {
class BrowserSettings;
}

class BrowserSettings : public QWidget
{
	Q_OBJECT
	
public:
	explicit BrowserSettings(QWidget *parent = 0);
	~BrowserSettings();

	// Location bar statuses
	struct Settings {
		bool showLocation;
		bool showNavigation;
		bool showLoading;
		bool showSettings;
		bool hideScrollbars;
		bool fallbackToDefaultUrl;
		bool fullscreen;
		bool dragOnTouch;
		/*! This is the last visited url */
		QString url;
		/*! This is the defualt url */
		QString defaultUrl;
		bool rememberLastUrl;
		bool wheelWorkaround;
		bool zoomToFit;
		bool showZoomButton;
		/*! When configuration is locked from cmd line use can't change it by UI */
		bool configurationLockedFromCmdLine;
		enum {
			ToolbarVisible,
			ToolbarHidden,
			ToolbarAuto
		} toolbarVisibility;
		/*! Exit pattern is used to match loaded url to exit application */
		QString exitPattern;
		bool exitButton;
		QString defaultUser;
		QString defaultPassword;
		bool exitOnHttpAuthFail;
		/*! Pin (actually md5 of pin) */
		QByteArray pin;
		bool enableCookies;
		int settingsTimeout;
		QString userAgent;
		int inactivityTimeout;
		int inactivityFromBacklightOffTimeout;
		int memAvailableRefreshMB;
		int memAvailableExitMB;
		int memAvailableExitFromBacklightOffMB;
		int reloadRetryTimeout;
		int numLoadRetries;
		bool disableJSTimeout;
		bool reloadOnJSTimeout;
		QString waitFlagFile;

		// Default initialization
		Settings()
			: showLocation(true)
			, showNavigation(true)
			, showLoading(true)
			, showSettings(true)
			, hideScrollbars(false)
			, fallbackToDefaultUrl(true)
			, fullscreen(true)
			, dragOnTouch(true)
			, url("")
			, defaultUrl("www.google.com")
			, rememberLastUrl(false)
			, wheelWorkaround(false)
			, zoomToFit(false)
			, showZoomButton(false)
			, configurationLockedFromCmdLine(false)
			, toolbarVisibility(ToolbarVisible)
			, exitPattern("")
			, exitButton(false)
			, exitOnHttpAuthFail(false)
			, pin("")
			, enableCookies(false)
			, settingsTimeout(5)
			, userAgent("WebkitBrowser")
			, disableJSTimeout(false)
			, reloadOnJSTimeout(false)
			, waitFlagFile()
		{}
	} settings;

	bool inputHasFocus();

public slots:
	/*! Init from qsettings */
	void initFromSettings();
	/*! Init from command line parameters */
	void initFromCommandLine();
	/*! Save modified settings to file */
	void saveSettings();

signals:
	void showNavigationToggled(bool);
	void showLoadingToggled(bool);
	void showSettingsToggled(bool);
	void showLocationToggled(bool);
	void touchNavigationToggled(bool);
	void closeClicked(bool);
	void fullScreenButtonToggled(bool);
	void zoomToFitToggled(bool);
	void showZoomButtonToggled(bool);
	void hideScrollbarsToggled(bool);
	
	void toolbarVisibilityChanged(int);

	/*! Url changed */
	void urlChanged(const QString& newUrl);
	/*! Timeout changed */
	void timeoutChanged(int newTimeout);
	
protected:
	void showEvent(QShowEvent* s);
	void hideEvent(QHideEvent* s);
	
	bool eventFilter(QObject *, QEvent *);

private slots:
	void on_lockSettingsButton_toggled(bool checked);
	
	void on_lockDialogBoxButtons_accepted();
	
	void on_lockDialogBoxButtons_rejected();
	
	void on_showPinCheckbox_toggled(bool checked);
	
	void toolbarVisibilityChangedSlot();
	

	// Apply settings to ui
	void applySettings();
	// Apply settings from ui
	void syncSettings();
	
	// Check pin
	bool checkPin(const QString& plainTextPin);
	
	
	void on_defaultUrl_editingFinished();
	void on_holdValue_editingFinished();

	void on_userAgentEdit_editingFinished();
	
private:
	
	Ui::BrowserSettings *ui;
	FlickCharm *m_flickCharm;
	bool m_validating;
	bool m_urlChanged;
	bool m_timeoutChanged;
};

#endif // SETTINGS_H
