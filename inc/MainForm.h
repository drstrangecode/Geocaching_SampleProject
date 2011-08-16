#ifndef _FORM1_H_
#define _FORM1_H_

#include <FBase.h>
#include <FUi.h>
#include <FIo.h>
#include <FSystem.h>
#include <FLocations.h>
#include <FGraphics.h>
#include <FLocales.h>
#include <FMedia.h>

class MainForm : public Osp::Ui::Controls::Form,
	public Osp::Ui::IActionEventListener,
	public Osp::Ui::ISlidableListEventListener,
	public Osp::Locations::ILocationListener
{

    class GeoCachedPoint {
        public:
            double latitude;
            double longitude;
            long long unixTimeStamp;
    };

// Construction
public:
	MainForm(void);
	virtual ~MainForm(void);
	bool Initialize(void);

// Implementation
protected:
	static const int ID_MENU_START = 1001;
	static const int ID_MENU_STOP = 1002;
	static const int ID_MENU_SHOW = 1003;
	static const int ID_MENU_FLUSH_DB = 1004;

	static const int LIST_ITEM_COORDS = 10001;
	static const int LIST_ITEM_TIME = 10002;
	static const int LIST_ITEM_ICON = 10003;

	static const int LIST_ITEM_HEIGHT = 90;

    static const int COORDS_TEXT_SIZE = 30;
    static const int TIMESTAMP_TEXT_SIZE = 26;

	Osp::Ui::Controls::OptionMenu *pOptionsMenu;
	Osp::Ui::Controls::SlidableList *pSlidableList;
	Osp::Ui::Controls::Label *pStatusLabel;

public:
	virtual result OnInitializing(void);
	virtual result OnTerminating(void);
	virtual void OnActionPerformed(const Osp::Ui::Control& source, int actionId);

	// ILocationListener methods
	virtual void OnLocationUpdated (Osp::Locations::Location &location);
	virtual void OnProviderStateChanged (Osp::Locations::LocProviderState newState);

	// ISlidableListEventListener
	virtual void OnListPropertyRequested (const Osp::Ui::Control &source);
	virtual void OnLoadToTopRequested(const Osp::Ui::Control &source, int index, int numItems);
	virtual void OnLoadToBottomRequested(const Osp::Ui::Control &source, int index, int numItems);
	virtual void OnUnloadItemRequested(const Osp::Ui::Control& source, int itemIndex);

private:

	Osp::Graphics::Bitmap * pListRowMarkerIcon;
	Osp::Locales::DateTimeFormatter * pDateTimeFormatter;

	void StartLocationUpdates();
	void StopLocationUpdates();

	bool CreateDatabaseIfNotExists();
	void InsertIntoDatabase(double lat, double lon);
	void GetPointFromDatabase(GeoCachedPoint * point, int pos);
	int GetStoredPointsCount();
	void RemoveAllPointsFromDatabase();

	Osp::Io::Database pGeoDatabase;
	Osp::Locations::LocationProvider pLocProvider;

	Osp::Ui::Controls::CustomListItemFormat *pSlidableListItemFormat;
	Osp::Ui::Controls::CustomListItem * CreateListItem(double lat, double lon, long long unixTimeStamp);


};

#endif	//_FORM1_H_
