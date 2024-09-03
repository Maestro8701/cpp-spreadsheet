
#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <stack>

class Cell::Impl {
    public:
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const {
            return {};
        }
        virtual bool HasCache() const {
            return true;
        }
        virtual void InvalidateCache() {}
        
    protected:
        Value value_;
        std::string text_;
    };

    class Cell::EmptyImpl : public Impl {
    public:
        Value GetValue() const override {
            return value_;
        }
        std::string GetText() const override {
            return text_;
        }
    };

    class Cell::TextImpl : public Impl {
    public:
        TextImpl(std::string_view text) {  
            text_ = text;
            if (text[0] == ESCAPE_SIGN) text = text.substr(1);
            value_ = std::string(text);
        }
        
        Value GetValue() const override {
            return value_;
        }
        std::string GetText() const override {
            return text_;
        }
    };

class Cell::FormulaImpl : public Impl {
public:
    FormulaImpl(std::string_view text, const SheetInterface& sheet) : sheet_(sheet) {
        formula_ptr_ = ParseFormula(std::move(std::string(text.substr(1))));
        text_ = FORMULA_SIGN + formula_ptr_->GetExpression(); 
    }  
    
    Value GetValue() const override {
        if (!cache_) cache_ = formula_ptr_->Evaluate(sheet_);
        auto value = formula_ptr_->Evaluate(sheet_);
        if (std::holds_alternative<double>(value)) return std::get<double>(value);
        return std::get<FormulaError>(value);
    }

    std::string GetText() const override {
        return text_;
    }

    bool HasCache() const override {
        return cache_.has_value();
    }

    void InvalidateCache() override {
        cache_.reset();
    }

    std::vector<Position> GetReferencedCells() const {
        return formula_ptr_->GetReferencedCells();
    }

private:
    std::unique_ptr<FormulaInterface> formula_ptr_;
    mutable std::optional<FormulaInterface::Value> cache_;
    const SheetInterface& sheet_;
};

Cell::Cell(Sheet& sheet) : impl_(std::make_unique<EmptyImpl>()), sheet_(sheet) {}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
    std::unique_ptr<Impl> impl;    
    if (text.empty()) {
        impl = std::make_unique<EmptyImpl>();
    }
    else if (text.size() > 1 && text[0] == FORMULA_SIGN) {
        impl = std::make_unique<FormulaImpl>(std::move(text), sheet_);
    }
    else impl = std::make_unique<TextImpl>(std::move(text));
    HasCircularDependency(*impl);  
    impl_ = std::move(impl);  
    for (Cell* outgoing : ref_cells_) {
        outgoing->dep_cells_.erase(this);
    }
    ref_cells_.clear();   
    auto ref_cells = impl_->GetReferencedCells();   
    RefCellsProcessing(ref_cells);
    InvalidateAllCache();
}

void Cell::Clear() {
    Set("");
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

bool Cell::HasCircularDependency(const Impl& impl) const {
    if (impl.GetReferencedCells().empty()) return false;
    
    std::unordered_set<const Cell*> ref_cells;
    for (const auto& pos : impl.GetReferencedCells()) {
        ref_cells.insert(sheet_.GetPtr(pos));
    }
    
    std::unordered_set<const Cell*> visited;
    std::stack<const Cell*> to_visit;
    to_visit.push(this);
    while (!to_visit.empty()) {
        const Cell* current = to_visit.top();
        to_visit.pop();
        visited.insert(current);
        if (ref_cells.find(current) != ref_cells.end()) {
            throw CircularDependencyException("Circular dependency detected");
        };       
        for (const Cell* cell : current->dep_cells_) {
            if (visited.find(cell) == visited.end()) to_visit.push(cell);
        }
    }
    return false;
}

void Cell::RefCellsProcessing(const std::vector<Position>& vec) {
        for (const auto& pos : vec) {
        Cell* outgoing = sheet_.GetPtr(pos);
        if (!outgoing) {
            sheet_.SetCell(pos, "");
            outgoing = sheet_.GetPtr(pos);
        }
        ref_cells_.insert(outgoing);
        outgoing->dep_cells_.insert(this);
    }
}

    void Cell::InvalidateAllCache() {
        if (impl_->HasCache()) {
            impl_->InvalidateCache();
            for (Cell* cell : dep_cells_) {
                cell->InvalidateAllCache();
            }
        }
    }

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}
