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

double HistoryData::getLastBolus()  {
    return list_bolus[list_bolus.size() - 1];
}
