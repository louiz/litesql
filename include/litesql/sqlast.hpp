#ifndef _litesql_sqlast_hpp_
#define _litesql_sqlast_hpp_
#include "litesql/sql.hpp"
namespace litesql {
    namespace sql {


        class Translator;

        /** Abstract syntax tree node */
        class AST {
        public:
            virtual ~AST() {}
            virtual std::string toString() const=0;
            virtual SQL* translate(Translator& t) = 0;
        };

        /** Raw SQL-injection */
        class Raw {
        public:
            virtual void accept(Visitor& v);
        };

        /** SQL SELECT statement */
        class Select : public Stmt {
        public:
            virtual void accept(Visitor& v);
        };

        /** SELECT result set field or expression */
        class Result : public AST {
        public:
            virtual void accept(Visitor& v);            
        };

        /** FROM-expression in SELECT-clause */
        class Sources : public AST {
        public:
            virtual void accept(Visitor& v);            
        };

        /** Single FROM-source in SELECT-clause */
        class FromSource : public AST {
        };

        /** FROM (table)-expression in SELECT-clause */
        class FromTable : public FromSource {
        };

        /** FROM (select)-expression in SELECT-clause */
        class FromSelect : public FromSource {
        };

        /** Base class for joins */
        class Join : public AST {
        };

        /** CROSS JOIN */
        class CrossJoin : public Join {
        };

        /** INNER JOIN _ ON _ */
        class InnerJoin : public Join {
        };

        /** LEFT OUTER JOIN _ ON _ */
        class LeftOuterJoin : public Join {
        };

        /** RIGHT OUTER JOIN _ ON _ */
        class RightOuterJoin : public Join {
        };

        /** FULL OUTER JOIN _ ON _ */
        class FullOuterJoin : public Join {
        };

        /** Base class for compound operations */
        class CompoundOp : public AST {
        };

        /** UNION SELECT _ */
        class Union : public CompoundOp {
        };

        /** INTERSECT SELECT _ */
        class Intersect : public CompoundOp {
        };

        /** EXCEPT SELECT _ */
        class Except : public CompoundOp {
        };
        
        /** ORDER BY-part of SELECT-statement */
        class OrderBy : public AST {

        };

        /** Base class for expressions */
        class Expr : public AST {
        };

        /** Base class for unary operator expressions */
        class UnOp : public Expr {
                    
        };

        /** NOT (_) */
        class Not : public UnOp {
        };

        /** -(_) */
        class UnaryMinus : public UnOp {
        };

        /** Base class for binary operator expressions */
        class BinOp : public Expr {
        };

        /** _ = _ */
        class Eq : public BinOp {
        };

        /** _ <> _ */
        class Neq : public BinOp {
        };

        /** _ < _ */
        class Lt : public BinOp {
        };

        /** _ <= _ */
        class LtEq : public BinOp {
        };

        /** _ > _ */
        class Gt : public BinOp {
        };

        /** _ >= _ */
        class GtEq : public BinOp {
        };

        /** (_) OR (_) */
        class Or : public BinOp {
        };

        /** (_) AND (_) */
        class And : public BinOp {
        };

        /** _ LIKE _ */
        class Like : public BinOp {
        };

        /** _ REGEXP _ */
        class RegExp : public BinOp {
        };

        /** _ IS NULL */
        class IsNull : public BinOp {
        };

        /** _ IN (_) */
        class In : public BinOp {
        };

        /** _ || _ */
        class Concat : public BinOp {
        };

        /** _ % _ */
        class Mod : public BinOp {
        };

        /** _ / _ */
        class Div : public BinOp {
        };

        /** _ * _ */
        class Mul : public BinOp {
        };

        /** _ + _ */
        class Add : public BinOp {
        };

        /** _ - _ */
        class Sub : public BinOp {
        };

        /** Converts a list of values to a list expression */
        class List : public BinOp {
        };

        /** Converts a value (int, string, whatever) to an expression */
        class ValueExpr : public Expr {
        };

        /** Value of a field of the result set-expression */
        class FieldExpr : public Expr {
        };


        /** The sequence used in the INSERT-operation */
        class FromSequence : public AST {
        };

        /** Atomic multi-table insert that will assign rows with
            same new unique identifier to multiple tables */
        class Insert : public Stmt {
        };

        /** Insertion to one table */
        class ToTable : public AST {
        };

        /** Assignment to a field */
        class Assign : public AST {
        };

        /** UPDATE statement */
        class Update : public Stmt {
        };

        /** DELETE statement */
        class Delete : public Stmt {
        };

        /** CREATE TABLE statement */
        class CreateTable : public Stmt {
        };

        /** DROP TABLE statement */
        class DropTable : public Stmt {
        };

        /** Field definition to be used in CREATE table statement */
        class Field : public AST {
        };

        /** Base class for field constraints */
        class FieldConstraint : public AST {
        };

        /** NOT NULL constraint for a field */
        class FieldNotNull : public FieldConstraint {
        };

        /** PRIMARY KEY constraint for a field */
        class FieldPrimaryKey : public FieldConstraint {
        };

        /** UNIQUE constraint for a field */
        class FieldUnique : public FieldConstraint {
        };

        /** CHECK constraint for a field */
        class FieldCheck : public FieldConstraint {
        };

        /** DEFAULT value for a field */
        class FieldDefault : public FieldConstraint {
        };

        /** FOREIGN KEY reference for a field */
        class FieldReference : public FieldConstraint {
        };

        /** Base class for actions */
        class Action : public AST {
        };

        /** CASCADE action (when a referenced row is deleted) */
        class Cascade : public Action {
        };

        /** SET NULL action (when a referenced row is deleted) */
        class SetNull : public Action {
        };

        /** Base class for table specific constraints */
        class TableConstraint : public AST {
        };

        /** multiple field PRIMARY KEY constraint */
        class TablePrimaryKey : public TableConstraint {
        };        

        /** CREATE INDEX statement */
        class CreateIndex : public Stmt {
        };

        /** DROP INDEX statement */
        class DropIndex : public Stmt {
        };

        /** AST translator base class */
        class Translator {
        public:
            virtual SQL* trnsRaw(Raw*);
            virtual SQL* trnsSelect(Select*);
            virtual SQL* trnsResult(Result*);
            virtual SQL* trnsSources(Sources*);
            virtual SQL* trnsFromSource(FromSource*);
            virtual SQL* trnsFromSelect(FromSelect*);
            virtual SQL* trnsCrossJoin(CrossJoin*);
            virtual SQL* trnsInnerJoin(InnerJoin*);
            virtual SQL* trnsLeftOuterJoin(LeftOuterJoin*);
            virtual SQL* trnsRightOuterJoin(RightOuterJoin*);
            virtual SQL* trnsFullOuterJoin(FullOuterJoin*);
            virtual SQL* trnsUnion(Union*);
            virtual SQL* trnsIntersect(Intersect*);
            virtual SQL* trnsExcept(Except*);
            virtual SQL* trnsOrderBy(OrderBy*);
            virtual SQL* trnsNot(Not*);
            virtual SQL* trnsUnaryMinus(UnaryMinus*);
            virtual SQL* trnsEq(Eq*);
            virtual SQL* trnsNeq(Neq*);
            virtual SQL* trnsLt(Lt*);
            virtual SQL* trnsLtEq(LtEq*);
            virtual SQL* trnsGt(Gt*);
            virtual SQL* trnsGtEq(GtEq*);
            virtual SQL* trnsOr(Or*);
            virtual SQL* trnsAnd(And*);
            virtual SQL* trnsLike(Like*);
            virtual SQL* trnsRegExp(RegExp*);
            virtual SQL* trnsIsNull(IsNull*);
            virtual SQL* trnsIn(In*);
            virtual SQL* trnsConcat(Concat*);
            virtual SQL* trnsMod(Mod*);
            virtual SQL* trnsDiv(Div*);
            virtual SQL* trnsMul(Mul*);
            virtual SQL* trnsAdd(Add*);
            virtual SQL* trnsSub(Sub*);
            virtual SQL* trnsList(List*);
            virtual SQL* trnsValueExpr(ValueExpr*);
            virtual SQL* trnsFieldExpr(FieldExpr*);
            virtual SQL* trnsFromSequence(FromSequence*);
            virtual SQL* trnsInsert(Insert*);
            virtual SQL* trnsToTable(ToTable*);
            virtual SQL* trnsAssign(Assign*);
            virtual SQL* trnsUpdate(Update*);
            virtual SQL* trnsDelete(Delete*);
            virtual SQL* trnsCreateTable(CreateTable*);
            virtual SQL* trnsDropTable(DropTable*);
            virtual SQL* trnsField(Field*);
            virtual SQL* trnsFieldConstraint(FieldConstraint*);
            virtual SQL* trnsFieldPrimaryKey(FieldPrimaryKey*);
            virtual SQL* trnsFieldUnique(FieldUnique*);
            virtual SQL* trnsFieldCheck(FieldCheck*);
            virtual SQL* trnsFieldDefault(FieldDefault*);
            virtual SQL* trnsFieldReference(FieldReference*);
            virtual SQL* trnsCascade(Cascade*);
            virtual SQL* trnsSetNull(SetNull*);
            virtual SQL* trnsTablePrimaryKey(TablePrimaryKey*);
            virtual SQL* trnsCreateIndex(CreateIndex*);
            virtual SQL* trnsDropIndex(DropIndex*);
        };

    }
}

#endif

