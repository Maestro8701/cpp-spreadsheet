#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>

class CellHasher {
public:
    size_t operator()(const Position p) const {
        return std::hash<std::string>()(p.ToString());
    }
};

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;
    
    const Cell* GetPtr(Position pos) const;
    Cell* GetPtr(Position pos);

private:
	std::unordered_map<Position, std::unique_ptr<Cell>, CellHasher> cells_;
    void IsPosValid(const Position& pos) const;
};
