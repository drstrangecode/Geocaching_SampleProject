#include "MainForm.h"

using namespace Osp::Base;
using namespace Osp::Ui;
using namespace Osp::Ui::Controls;
using namespace Osp::Io;
using namespace Osp::System;
using namespace Osp::Locations;
using namespace Osp::Graphics;
using namespace Osp::Media;
using namespace Osp::Locales;

MainForm::MainForm(void) {
}

MainForm::~MainForm(void) {
}

bool MainForm::Initialize() {
    // Construct an XML form
    Construct(L"IDF_MAINFORM");

    /* Create/Open the database */
    CreateDatabaseIfNotExists();

    /* Build the locations provider */
    pLocProvider.Construct(LOC_METHOD_GPS);

    return true;
}

result MainForm::OnInitializing(void) {
    result r = E_SUCCESS;

    /* Initialize the status label */
    pStatusLabel = static_cast<Label *> (GetControl(L"IDC_STATUS_LABEL"));
    pStatusLabel->SetText(L"Ready");

    /* Create the OptionMenu */
    pOptionsMenu = new OptionMenu();
    pOptionsMenu->Construct();
    pOptionsMenu->AddItem("Start", ID_MENU_START);
    pOptionsMenu->AddItem("Stop", ID_MENU_STOP);
    pOptionsMenu->AddItem("Flush Database", ID_MENU_FLUSH_DB);
    pOptionsMenu->AddActionEventListener(*this);

    SetOptionkeyActionId(ID_MENU_SHOW);
    AddOptionkeyActionListener(*this);

    /* Configure the format of custom list rows (CustomListItem) */
    pSlidableListItemFormat = new CustomListItemFormat();
    pSlidableListItemFormat->Construct();
    pSlidableListItemFormat->AddElement(LIST_ITEM_ICON,
                             Osp::Graphics::Rectangle(5, 5, 80, 80));
    pSlidableListItemFormat->AddElement(LIST_ITEM_COORDS,
                             Osp::Graphics::Rectangle(90, 10, 460, 30),
                             COORDS_TEXT_SIZE,
                             Color::COLOR_WHITE,
                             Color::COLOR_BLACK);
    pSlidableListItemFormat->AddElement(LIST_ITEM_TIME,
                             Osp::Graphics::Rectangle(90, 55, 460, 30),
                             TIMESTAMP_TEXT_SIZE,
                             Color::COLOR_GREY,
                             Color::COLOR_BLACK);

    /* Configure the slidable list */
    pSlidableList = static_cast<SlidableList *> (GetControl(L"IDC_SLIDABLE_LIST"));
    pSlidableList->AddSlidableListEventListener(*this);

    /* Initialize the marker icon used in the list rows */
    Image image;
    image.Construct();
    String icoPath = L"/Home/mark.png";
    pListRowMarkerIcon = image.DecodeN(icoPath, BITMAP_PIXEL_FORMAT_ARGB8888, 80, 80);

    /* Initialize the DateTimeFormatter used to print timestamp in the list rows */
    String customizedPattern = L"HH:mm:ss - dd MMM yyyy";
    LocaleManager localeManager;
    localeManager.Construct();
    Locale systemLocale = localeManager.GetSystemLocale();
    pDateTimeFormatter = DateTimeFormatter::CreateDateFormatterN(systemLocale);
    pDateTimeFormatter->ApplyPattern(customizedPattern);

    return r;
}

result MainForm::OnTerminating(void) {
    result r = E_SUCCESS;

    delete pOptionsMenu;
    delete pSlidableListItemFormat;
    delete pListRowMarkerIcon;
    delete pDateTimeFormatter;

    return r;
}

void MainForm::OnActionPerformed(const Osp::Ui::Control& source, int actionId) {
    switch (actionId) {
        case ID_MENU_START: {
            StartLocationUpdates();
            pStatusLabel->SetText(L"Waiting for GPS position...");
            pStatusLabel->RequestRedraw(true);
        }
        break;
        case ID_MENU_STOP: {
            StopLocationUpdates();
            pStatusLabel->SetText(L"Stopped");
            pStatusLabel->RequestRedraw(true);
        }
        break;
        case ID_MENU_FLUSH_DB: {
            RemoveAllPointsFromDatabase();
            pStatusLabel->SetText(L"Database flushed");
            pSlidableList->RemoveAllItems();
            RequestRedraw(true);

        }
        break;
        case ID_MENU_SHOW: {
            /* OptionKey has been tapped */
            AppLog("ID_MENU_SHOW\n");
            pOptionsMenu->SetShowState(true);
            pOptionsMenu->Show();
        }
        break;
        default:
        break;
    }
}

void MainForm::StartLocationUpdates() {
    AppLog("StartLocationUpdates()\n");

    int interval = 5;
    pLocProvider.RequestLocationUpdates(*this, interval, false);

    RequestRedraw();
}

void MainForm::StopLocationUpdates() {
    AppLog("StopLocationUpdates()\n");

    pLocProvider.CancelLocationUpdates();

    RequestRedraw();
}

/*
 * Locations listener methods
 * */
void MainForm::OnLocationUpdated(Osp::Locations::Location &location) {

    if (location.IsValid()) {
        InsertIntoDatabase(location.GetQualifiedCoordinates()->GetLatitude(), location.GetQualifiedCoordinates()->GetLongitude());
        String status;
        status.Format(50, L"Recorded %d points", GetStoredPointsCount());
        pStatusLabel->SetText(status);
        pSlidableList->RemoveAllItems();
        RequestRedraw(true);

    }

}

void MainForm::OnProviderStateChanged(Osp::Locations::LocProviderState newState) {
    switch (newState) {
        case LOC_PROVIDER_AVAILABLE:
            AppLog("Locations provider available");
            break;
        case LOC_PROVIDER_OUT_OF_SERVICE:
            AppLog("Locations provider not available");
            break;
        case LOC_PROVIDER_TEMPORARILY_UNAVAILABLE:
            AppLog("Signal lost, please go outside");
            break;
    }
}

/*
 * Database handling methods
 * */

bool MainForm::CreateDatabaseIfNotExists() {
    /* Construct the database if it does not exists */
    String pDatabaseName(L"/Home/geo_points.db");

    String sql;
    result r = E_SUCCESS;

    /* Create the database if it does not exist */
    r = pGeoDatabase.Construct(pDatabaseName, true);
    if (IsFailed(r)) return false;

    sql.Append(L"CREATE TABLE IF NOT EXISTS geo_point ( \
                     id INTEGER PRIMARY KEY AUTOINCREMENT, \
                     latitude DOUBLE, \
                     longitude DOUBLE, \
                     unix_timestamp INTEGER);");

    r = pGeoDatabase.ExecuteSql(sql, true);
    if (IsFailed(r)) AppLog("Error: %s", GetErrorMessage(r));

    AppLog("Successfully created/opened the DB");

    return true;

}

void MainForm::InsertIntoDatabase(double lat, double lon) {

    AppLog("InsertIntoDatabase(%.6f, %.6f)\n", lat, lon);

    long long unixTimestamp;
    SystemTime::GetTicks(unixTimestamp);

    unixTimestamp /= 1000;

    String statement;
    statement.Append(L"INSERT INTO geo_point (latitude, longitude, unix_timestamp) VALUES (?, ?, ?);");
    DbStatement* pStmt = pGeoDatabase.CreateStatementN(statement);

    pGeoDatabase.BeginTransaction();

    /* Bind the actual values that are going to be written into the new db record */

    pStmt->BindDouble(0, lat);
    pStmt->BindDouble(1, lon);
    pStmt->BindInt64(2, unixTimestamp);

    DbEnumerator * pEnum = pGeoDatabase.ExecuteStatementN(*pStmt);
    int r = GetLastResult();
    if (IsFailed(r)) AppLog("Error: %s", GetErrorMessage(r));

    pGeoDatabase.CommitTransaction();

    if (pEnum) delete pEnum;
    delete pStmt;
}

void MainForm::GetPointFromDatabase(GeoCachedPoint * point, int pos) {

    String statement;
    statement.Append(L"SELECT latitude, longitude, unix_timestamp FROM geo_point LIMIT ?,1");
    DbStatement* pStmt = pGeoDatabase.CreateStatementN(statement);

    pStmt->BindInt(0, pos);

    int r = GetLastResult();
    if (IsFailed(r)) AppLog("Error: %s on parameter %d", GetErrorMessage(r), pos);

    DbEnumerator* pEnum = pGeoDatabase.ExecuteStatementN(*pStmt);
    r = GetLastResult();
    if (IsFailed(r)) AppLog("Error: %s", GetErrorMessage(r));

    if (pEnum == 0) {
        delete pStmt;
        return;
    }

    while(pEnum->MoveNext() == E_SUCCESS) {
        pEnum->GetDoubleAt(0, point->latitude);
        pEnum->GetDoubleAt(1, point->longitude);
        pEnum->GetInt64At(2, point->unixTimeStamp);
    }

    if (pEnum) delete pEnum;
    delete pStmt;

}

int MainForm::GetStoredPointsCount() {

    String statement;
    statement.Append(L"SELECT COUNT(*) FROM geo_point");
    DbStatement* pStmt = pGeoDatabase.CreateStatementN(statement);

    DbEnumerator* pEnum = pGeoDatabase.ExecuteStatementN(*pStmt);
    int r = GetLastResult();
    if (IsFailed(r)) AppLog("Error: %s", GetErrorMessage(r));

    if (pEnum == 0) {
        delete pStmt;
        return 0;
    }

    int count = 0;

    while(pEnum->MoveNext()== E_SUCCESS) {
        pEnum->GetIntAt(0, count);
    }

    if (pEnum) delete pEnum;
    delete pStmt;

    return count;

}

void MainForm::RemoveAllPointsFromDatabase() {
    AppLog("RemoveAllPointsFromDatabase()");

    String statement;
    statement.Append(L"DELETE FROM geo_point");
    DbStatement* pStmt = pGeoDatabase.CreateStatementN(statement);

    pGeoDatabase.BeginTransaction();

    DbEnumerator * pEnum = pGeoDatabase.ExecuteStatementN(*pStmt);

    pGeoDatabase.CommitTransaction();

    if (pEnum) delete pEnum;
    delete pStmt;

}

// ISlidableListEventListener

CustomListItem* MainForm::CreateListItem(double lat, double lon, long long unixTimeStamp) {
    CustomListItem* pItem = new CustomListItem();
    pItem->Construct(LIST_ITEM_HEIGHT);

    /*
     * Convert unix-timestamp to DateTime
     * Get Ticks gets current system time in milliseconds since 1/1/1970. But DateTime
     * represents from January 1 1 00:00:00". So we need to add the time difference
     * to get the current time using date time
     * */
    DateTime diffDateTime;
    diffDateTime.SetValue(1970, 1, 1, 00, 00, 00);

    TimeSpan ts1 = diffDateTime.GetTime();

    TimeSpan ts(unixTimeStamp * 1000);
    ts = ts + ts1;

    DateTime convertedDateTime;
    convertedDateTime.SetValue(ts);

    String coordsText;
    String timeText;

    /* Stored time is utc, so let's convert it to the system timezone */
    LocaleManager locManager;
    locManager.Construct();
    TimeZone timeZone = locManager.GetSystemTimeZone();
    convertedDateTime = timeZone.UtcTimeToWallTime(convertedDateTime);

    /* Format the timestamp */
    pDateTimeFormatter->Format(convertedDateTime, timeText);

    /* Format coordinates data */
    coordsText.Format(100, L"Lat: %.6f Lon: %.6f", lat, lon);

    pItem->SetItemFormat(*pSlidableListItemFormat);
    pItem->SetElement(LIST_ITEM_ICON, *pListRowMarkerIcon, pListRowMarkerIcon);
    pItem->SetElement(LIST_ITEM_COORDS, coordsText);
    pItem->SetElement(LIST_ITEM_TIME, timeText);

    return pItem;
}

void MainForm::OnListPropertyRequested(const Osp::Ui::Control &source) {
    AppLog("OnListPropertyRequested()");
    // Get the number of items to display
    int itemsCount = GetStoredPointsCount();
    int itemHeight = LIST_ITEM_HEIGHT;
    pSlidableList->SetItemCountAndHeight(itemsCount, itemsCount * itemHeight);

}

void MainForm::OnLoadToTopRequested(const Osp::Ui::Control &source, int index, int numItems) {

    for (int i = index; i > index - numItems; i--) {
        GeoCachedPoint point;
        GetPointFromDatabase(&point, (pSlidableList->GetItemCount() - i) - 1);
        pSlidableList->LoadItemToTop(*CreateListItem(point.latitude, point.longitude, point.unixTimeStamp), i + 1);
    }

}

void MainForm::OnLoadToBottomRequested(const Osp::Ui::Control &source, int index, int numItems) {

    for (int i = index; i < index + numItems; i++) {
        GeoCachedPoint point;
        GetPointFromDatabase(&point, (pSlidableList->GetItemCount() - i) - 1);
        pSlidableList->LoadItemToBottom(*CreateListItem(point.latitude, point.longitude, point.unixTimeStamp), i + 1);
    }

}

void MainForm::OnUnloadItemRequested(const Osp::Ui::Control& source, int itemIndex) {

}
