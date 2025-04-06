#include "history_data.h"

HistoryData::HistoryData(QObject *parent): QObject(parent) {
    historyIndex = 0;
    history_size = 0;
}

int HistoryData::getHistorySize(){
    return history_size;
}

int HistoryData::getHistoryIndex(){
    return historyIndex;
}

void HistoryData::setHistorySize(int size){
    history_size = size;
}

void HistoryData::setHistoryIndex(int index){
    historyIndex = index;
}

void HistoryData::addEntryDateTime(QString data){
    list_datetime << data;
}

void HistoryData::addEntryBasal(double data){
    list_basal << data;
}

void HistoryData::addEntryBolus(double data){
    list_bolus << data;
}

void HistoryData::addEntryInsulin(double data){
    list_insulin << data;
}

void HistoryData::addEntryCorrection(double data){
    list_correction << data;
}

void HistoryData::addEntryCarbs(double data){
    list_carbs << data;
}

void HistoryData::addEntryGlucose(double data){
    list_glucose << data;
}

QString HistoryData::getDateTime(int index){
    return list_datetime[index];
}

double HistoryData::getBasal(int index){
    return list_basal[index];
}

double HistoryData::getBolus(int index){
    return list_bolus[index];
}

double HistoryData::getInsulin(int index){
    return list_insulin[index];
}

double HistoryData::getCorrection(int index){
    return list_correction[index];
}

double HistoryData::getCarbs(int index){
    return list_carbs[index];
}

double HistoryData::getGlucose(int index){
    return list_glucose[index];
}

QString HistoryData::getLastDateTime()  {
    if(list_datetime.size() == 0){
        return "0";
    }
    else{
        return list_datetime[list_datetime.size() - 1];
    }
}

double HistoryData::getLastBolus()  {
    if(list_bolus.size() == 0){
        return 0;
    }
    else{
        return list_bolus[list_bolus.size() - 1];
    }
}

bool HistoryData::isBasalEmpty(){
    if(list_bolus.size() == 0){
        return true;
    }
    else{
        return false;
    }
}

double HistoryData::getBasalVariation(){
    if(list_basal.size() == 0){
        return 0;
    }
    else if(list_basal.size() == 1){
        return list_basal[list_basal.size() - 1];
    }
    else{
        return list_basal[list_basal.size() - 1] - list_basal[list_basal.size() - 2];
    }
}

double HistoryData::getCarbsVariation(){
    if(list_carbs.size() == 0){
        return 0;
    }
    else if(list_carbs.size() == 1){
        return list_carbs[list_carbs.size() - 1];
    }
    else{
        return list_carbs[list_carbs.size() - 1] - list_carbs[list_carbs.size() - 2];
    }
}

double HistoryData::getGlucoseVariation(){
    if(list_glucose.size() == 0){
        return 0;
    }
    else if(list_glucose.size() == 1){
        return list_glucose[list_glucose.size() - 1];
    }
    else{
        return list_glucose[list_glucose.size() - 1] - list_glucose[list_glucose.size() - 2];
    }
}
