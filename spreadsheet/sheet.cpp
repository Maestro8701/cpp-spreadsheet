#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <iostream>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) throw InvalidPositionException("Can't set invalid cell");
    if (!cells_.count(pos)) cells_[pos] = std::make_unique<Cell>(*this);
    cells_[pos]->Set(std::move(text));
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return GetPtr(pos);
}

CellInterface* Sheet::GetCell(Position pos) {
    return GetPtr(pos);
}

void Sheet::ClearCell(Position pos) {
    IsPosValid(pos);
    if (cells_.count(pos)) cells_.at(pos) = nullptr;      
}

Size Sheet::GetPrintableSize() const {
    Size result{ 0, 0 };    
    for (auto it = cells_.begin(); it != cells_.end(); ++it) {
        if (it->second != nullptr) {
            result.rows = std::max(result.rows, it->first.row + 1);
            result.cols = std::max(result.cols, it->first.col + 1);
        }
    }
    return result;
}

void Sheet::PrintValues(std::ostream& output) const {
    Size size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {
        for (int col = 0; col < size.cols; ++col) {
            if (col > 0) output << "\t";
            const auto& it = cells_.find({ row, col });
            if (it != cells_.end() && it->second != nullptr && !it->second->GetText().empty()) {
                std::visit([&](const auto value) { output << value; }, it->second->GetValue());
            }
        }
        output << "\n";
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    Size size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {
        for (int col = 0; col < size.cols; ++col) {
            if (col > 0) output << "\t";
            const auto& it = cells_.find({ row, col });
            if (it != cells_.end() && it->second != nullptr && !it->second->GetText().empty()) {
                output << it->second->GetText();
            }
        }
        output << "\n";
    }
}

const Cell* Sheet::GetPtr(Position pos) const {
    if (!pos.IsValid()) throw InvalidPositionException("Can't get invalid cell position");
    const auto cell = cells_.find(pos);
    if (cell == cells_.end()) return nullptr;
    return cells_.at(pos).get();
}

Cell* Sheet::GetPtr(Position pos) {
    return const_cast<Cell*>(
        static_cast<const Sheet&>(*this).GetPtr(pos));
}

void Sheet::IsPosValid(const Position& pos) const {
    if (!pos.IsValid()) throw InvalidPositionException("Can't clear invalid cell position");
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
