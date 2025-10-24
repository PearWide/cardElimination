#include "UndoModel.h"
#include "cocos2d.h"

USING_NS_CC;

UndoModel::UndoModel(size_t maxSteps)
    : _maxSteps(maxSteps) {
    CCLOG("UndoModel created");
}

UndoModel::~UndoModel() {
    CCLOG("UndoModel destroyed");
}

void UndoModel::addStep(const UndoStep& step) {
    _undoSteps.push_back(step);
    trimSteps();
    CCLOG("Step added");
}

UndoStep UndoModel::popStep() {
    if (_undoSteps.empty()) {
        return UndoStep();
    }

    UndoStep step = _undoSteps.back();
    _undoSteps.pop_back();
    return step;
}

bool UndoModel::canUndo() const {
    return !_undoSteps.empty();
}

size_t UndoModel::getStepCount() const {
    return _undoSteps.size();
}

void UndoModel::clear() {
    _undoSteps.clear();
    CCLOG("UndoModel cleared");
}

void UndoModel::setMaxSteps(size_t maxSteps) {
    _maxSteps = maxSteps;
    trimSteps();
}

void UndoModel::trimSteps() {
    if (_undoSteps.size() > _maxSteps) {
        _undoSteps.erase(_undoSteps.begin(), _undoSteps.begin() + (_undoSteps.size() - _maxSteps));
    }
}