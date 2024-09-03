#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>
#include <optional>    

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override; 
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    
private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;

    bool HasCircularDependency(const Impl& impl) const;
    void RefCellsProcessing(const std::vector<Position>& vec);
    void InvalidateAllCache();

    std::unique_ptr<Impl> impl_;
    Sheet& sheet_;
    std::unordered_set<Cell*> dep_cells_; // зависимые ячейки
    std::unordered_set<Cell*> ref_cells_; // используемые ячейки
    mutable std::optional<Value> cache_;
};
