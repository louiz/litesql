#include "litesql/split.hpp"
#include "litesql/types.hpp"
#include "litesql-gen-cpp.hpp"
#include "litesql-gen-main.hpp"
#include "cppgenerator.hpp"
#include "xmlobjects.hpp"
#include <ctype.h>
#include <cstdio>
#include <map>
using namespace std;
using namespace gen;
using namespace litesql;



string quote(string s) {
    return "\"" + s + "\"";
}
string brackets(string s) {
    return "(" + s + ")";
}
static bool validID(string s) {
    static const char* words[] = 
        {"asm","break","case","catch",
         "char","class","const","continue","default",
         "delete","do","double","else","enum","extern",
         "float","for","friend","goto","if","inline","int",
         "long","new","operator","private","protected",
         "public","register","return","short","signed",
         "sizeof","static","struct","switch","template",
         "this","throw","try","typedef","union","unsigned",
         "virtual","void","volatile","while",   
         // LiteSQL specific
         "initValues", "insert", "addUpdates", "addIDUpdates",
         "getFieldTypes", "delRecord", "delRelations",
         "update", "del", "typeIsCorrect", "upcast", "upcastCopy"
        };

    for (size_t i = 0; i < sizeof(words) / sizeof(words[0]); i++)
        if (s == words[i])
            return false;
    return true; 
}
static void sanityCheck(xml::Database& db,
                        vector<xml::Object*>& objects,
                        vector<xml::Relation*>& relations) {
    using namespace litesql;
    if (!validID(db.name)) 
        throw Except("invalid id: database.name : " + db.name);
    for (size_t i = 0; i < objects.size(); i++) {
        xml::Object& o = *objects[i];
        if (!validID(o.name))
            throw Except("invalid id: object.name : " + o.name);
        for (size_t i2 = 0; i2 < o.fields.size(); i2++) {
            xml::Field& f = *o.fields[i2];
            if (!validID(f.name))
                throw Except("invalid id: object.field.name : " + f.name);
        }
    }
    for (size_t i = 0; i < relations.size(); i++) {
        xml::Relation& r = *relations[i];
        if (!validID(r.getName()))
            throw Except("invalid id: relation.name : " + r.getName());
        for (size_t i2 = 0; i2 < r.fields.size(); i2++) {
            xml::Field& f = *r.fields[i2];
            if (!validID(f.name))
                throw Except("invalid id: relation.field.name : " + f.name);
        }
        for (size_t i2 = 0; i2 < r.related.size(); i2++) {
            xml::Relate& rel = *r.related[i2];
            if (!validID(rel.handle) && !rel.handle.empty())
                throw Except("invalid id: relation.relate.handle : " + rel.handle);
        }
    }   
}


void writeStaticObjData(Class& cl, const xml::Object& o) {
    Variable type__("type__", "const std::string", quote(o.name));
    type__.static_();
    cl.variable(type__);

    Variable table__("table__", "const std::string", 
                     quote(o.getTable()));
    table__.static_();
    cl.variable(table__);

    if (!o.parentObject) {
        Variable sequence__("sequence__", "const std::string", 
                            quote(o.getSequence()));
        sequence__.static_();
        cl.variable(sequence__);
    }
}
void writeObjFields(Class & cl, const xml::Object & o) {
    Method init("initValues", "void");
    bool hasValues = false;
    string ftypeClass ="const litesql::FieldType";

    init.static_();

    Class ownData("Own");
    Variable ftype("Id", ftypeClass, 
                   quote("id_") + "," + quote("INTEGER") 
                   + "," + quote(o.getTable()));
    ftype.static_();
    ownData.variable(ftype);
    
    cl.class_(ownData);
  
    for (size_t i = 0; i < o.fields.size(); i++) {
        const xml::Field& fld = *o.fields[i];
        string data = quote(fld.name + "_") + "," +
            quote(fld.getSQLType()) + "," +
            "table__";
        if (!fld.values.empty()) {
            data += "," + fld.name + "_values"; 
            Variable values(fld.name + "_values", 
                    "std::vector < std::pair< std::string, std::string > >");
            values.static_().protected_();
            cl.variable(values);
            hasValues = true;
            init.body(fld.name + "_values.clear();");
            for (size_t i2 = 0; i2 < fld.values.size(); i2++) {
                const xml::Value& val = fld.values[i2];
                init.body(fld.name + "_values.push_back(make_pair<std::string, std::string>("
                        + quote(val.name) + "," + quote(val.value) + "));");    
            }
        }
        if (!fld.values.empty()) {
            ftypeClass = fld.fieldTypeName + "Type";
            Class ftypeCl(ftypeClass, "litesql::FieldType");
            Method cons(ftypeClass);
            ftypeClass = "const " + o.name + "::" + ftypeClass;
            cons.param(Variable("n", "const std::string&"))
                .param(Variable("t", "const std::string&"))
                .param(Variable("tbl", "const std::string&"))
                .param(Variable("vals", "const litesql::FieldType::Values&", "Values()"))
                .constructor("litesql::FieldType(n,t,tbl,vals)");

            ftypeCl.method(cons);
            for (size_t v = 0; v < fld.values.size(); v++) {
                const xml::Value& value = fld.values[v];
                string v;
                if (fld.getCPPType() == "std::string")
                    v = quote(value.value);
                else
                    v = value.value;
                Variable val(value.name, "const " + fld.getCPPType(), v);

                val.static_();
                ftypeCl.variable(val);
            }
            cl.class_(ftypeCl);
        }

        Variable ftype(fld.fieldTypeName, ftypeClass, data);
        ftype.static_();
        Variable field(fld.name, "litesql::Field<" + fld.getCPPType() + ">");
        cl.variable(ftype);
        cl.variable(field);
        if (fld.values.size() > 0) {
            Class valueHolder(xml::capitalize(fld.name));
            for (size_t v = 0; v < fld.values.size(); v++) {
                const xml::Value& value = fld.values[v];
                string v;
                if (fld.getCPPType() == "std::string")
                    v = quote(value.value);
                else
                    v = value.value;
                Variable val(value.name, "const " + fld.getCPPType(), v);

                val.static_();
                valueHolder.variable(val);
            }
            cl.class_(valueHolder);
        }
    }
    if (hasValues)
        cl.method(init);
}
void writeObjConstructors(Class& cl, const xml::Object& o) {
    Method defaults("defaults", "void");
    defaults.protected_();
    bool hasDefaults = false;
    for (size_t i = 0; i < o.fields.size(); i++) {
        const xml::Field& f = *o.fields[i];
        if (!f.default_.empty() || !f.hasQuotedValues())
            defaults.body(f.name + " = " + f.getQuotedDefaultValue() + ";");
            hasDefaults = true;
    } 

    
    Method cons1(o.name); // Object(const Database &)
    string consParams = o.inherits + "(db)";
    string cons2Params = o.inherits + "(db, rec)";
    if (o.fields.size() > 0) {
        Split fieldInst;
        for (size_t i = 0; i < o.fields.size(); i++) {
            const xml::Field& f = *o.fields[i];
            fieldInst.push_back(f.name + brackets(f.fieldTypeName));
        }
        consParams += ", " + fieldInst.join(", ");
        cons2Params += ", " + fieldInst.join(", ");

    }

    cons1.param(Variable("db", "const litesql::Database&"))
        .constructor(consParams);
    if (hasDefaults)
        cons1.body("defaults();");
    
    Method cons2(o.name); // Object(const Database &, const Record& row
    cons2.param(Variable("db", "const litesql::Database&"))
        .param(Variable("rec", "const litesql::Record&"))
        .constructor(cons2Params);
    if (hasDefaults) 
        cons2.body("defaults();");
    if (o.fields.size() > 0) {
        int last = o.getLastFieldOffset();
        cons2.body("size_t size = "
                   "(rec.size() > " + toString(last) + ")"
                   " ? " + toString(last) + " : rec.size();")
            .body("switch(size) {");

        for(int i = o.fields.size() - 1; i >= 0; i--) {
            int p = last - o.fields.size() + i;
            cons2.body("case " + toString(p+1) + ": " 
                       + o.fields[i]->name 
                       + " = convert<const std::string&, "
                       + o.fields[i]->getCPPType() 
                       + ">(rec[" + toString(p) + "]);")
                .body("    " + o.fields[i]->name + ".setModified(false);");

        }
        cons2.body("}");
    }
    
    Method cons3(o.name); // Object(const Object& obj);
    string consParams3 = o.inherits + "(obj)";    
    if (o.fields.size() > 0) {
        Split fieldCopy;
        for (size_t i = 0; i < o.fields.size(); i++) {
            const xml::Field& f = *o.fields[i];
            fieldCopy.push_back(f.name + brackets("obj." + f.name));
        }
        consParams3 += ", " + fieldCopy.join(", ");
    }
    cons3.param(Variable("obj", "const " + o.name + "&"))
        .constructor(consParams3);
    if (hasDefaults)
        cl.method(defaults);
    
    Method assign("operator=", "const " + o.name + "&");
    assign.param(Variable("obj", "const " + o.name + "&"));
    if (o.fields.size() > 0) {
        assign.body("if (this != &obj) {");
        for (size_t i = 0; i < o.fields.size(); i++) {
            const xml::Field& f = *o.fields[i];
            assign.body("    " + f.name + " = obj." + f.name + ";");
        }
        assign.body("}");
    }
    assign.body(o.inherits + "::operator=(obj);");
    assign.body("return *this;");
    
    cl.method(cons1).method(cons2).method(cons3).method(assign);
    
}
void writeObjRelationHandles(Class& cl, xml::Object& o) {
    for (size_t i = 0; i < o.handles.size(); i++) {
        xml::RelationHandle& handle = *o.handles[i];
        xml::Relation* rel = handle.relation;

        string className = xml::capitalize(handle.name) + "Handle";
        Class hcl(className,
                  "litesql::RelationHandle<" + o.name + ">");
        Method cons(className);
        cons.param(Variable("owner", "const " + o.name + "&")).
            constructor("litesql::RelationHandle<" + o.name + ">(owner)");
        
        Method link("link", "void");
        Method unlink("unlink", "void");
        Split params;
        params.push_back("owner->getDatabase()");
        params.resize(1 + rel->related.size());
        params[1 + handle.relate->paramPos] = "*owner";

        for (size_t i2 = 0; i2 < handle.destObjects.size(); i2++) {
            xml::Object* dest = handle.destObjects[i2].first;
            xml::Relate * relate = handle.destObjects[i2].second;
            Variable var("o" + toString(i2), "const " + dest->name + "&");
            link.param(var);
            unlink.param(var);
            params[1 + relate->paramPos] = "o" + toString(i2);
        }


        for (size_t i2 = 0; i2 < rel->fields.size(); i2++) {
            xml::Field& field = *rel->fields[i2];
            // FIXME: default-arvoiset parametrit viimeiseksi

            link.param(Variable(field.name, field.getCPPType(), 
                                field.getQuotedDefaultValue()));
            unlink.param(Variable(field.name, field.getCPPType()));
            params.push_back(field.name);
        }
        link.body(rel->getName() + "::link(" + params.join(", ") + ");");
        unlink.body(rel->getName() + "::unlink(" + params.join(", ") + ");");
        Variable exprParam("expr", "const litesql::Expr&", "litesql::Expr()");
        Variable srcExprParam("srcExpr", "const litesql::Expr&", "litesql::Expr()");
        Method del("del", "void");
        params.clear();
        params.push_back("owner->getDatabase()");
        params.push_back("expr && " + rel->getName() 
                + "::" + handle.relate->fieldTypeName + " == owner->id");
        del.param(exprParam).body(rel->getName() + "::del(" + params.join(", ") + ");");


        hcl.method(cons).method(link).method(unlink).method(del);
        string extraExpr = " && srcExpr";
        if (handle.destObjects.size() == 1) {
            xml::Object* dest = handle.destObjects[0].first;
            xml::Relate* relate = handle.destObjects[0].second;
            Method get("get", "litesql::DataSource<" + dest->name + ">");
            get.param(exprParam).param(srcExprParam);
            
            params.clear();
            params.push_back("owner->getDatabase()");
            params.push_back("expr");
            params.push_back("(" + rel->getName() + "::" + handle.relate->fieldTypeName + " == owner->id)"
                             + extraExpr);
            
            get
                .body("return " + rel->getName() + "::" + relate->getMethodName
                      + brackets(params.join(", ")) + ";");
            
            hcl.method(get);
        } else {
            if (rel->sameTypes() <= 2) {
                Method getTpl("get", "litesql::DataSource<T>");
                getTpl.template_("class T").defineOnly()
                    .param(exprParam).param(srcExprParam);
                hcl.method(getTpl);                
                for (size_t i2 = 0; i2 < handle.destObjects.size(); i2++) {
                    xml::Object* dest = handle.destObjects[i2].first;
                    xml::Relate* relate = handle.destObjects[i2].second;
                    Method get("get", 
                               "litesql::DataSource<" + dest->name + ">");
                    get.templateSpec("").param(exprParam).param(srcExprParam);
                    params.clear();
                    params.push_back("owner->getDatabase()");
                    params.push_back("expr");
                    params.push_back("(" + rel->getName() + "::" + handle.relate->fieldTypeName + " == owner->id)"
                                     + extraExpr);
                    get.body("return " + rel->getName() + "::" + relate->getMethodName
                              + brackets(params.join(", ")) + ";");
                    hcl.method(get); 
                }    
            } else {
                for (size_t i2 = 0; i2 < handle.destObjects.size(); i2++) {
                    xml::Object* dest = handle.destObjects[i2].first;
                    xml::Relate* relate = handle.destObjects[i2].second;
                    string num = toString(i2 + 1);
                    Method get("get" + dest->name + num, 
                               "litesql::DataSource<" + dest->name + ">");
                    get.param(exprParam).param(srcExprParam);
                    params.clear();
                    params.push_back("owner->getDatabase()");
                    params.push_back("expr");
                    params.push_back("(" + rel->getName() + "::" + handle.relate->fieldTypeName + " == owner->id)"
                                     + extraExpr);
                    get.body("return " + rel->getName() + "::" + relate->getMethodName
                              + brackets(params.join(", ")) + ";");
                    hcl.method(get);
                }
            }
        }
        Method getRows("getRows", "litesql::DataSource<" 
                       + rel->getName() + "::Row>");
        getRows.param(exprParam)
            .body("return " + rel->getName()
                  + "::getRows(owner->getDatabase(), "
                  "expr && (" + rel->getName() + "::" 
                  + handle.relate->fieldTypeName +
                  " == owner->id));");
        hcl.method(getRows);
        cl.class_(hcl);
        Method hdlMethod(handle.name, o.name + "::" + className);
        hdlMethod.body("return " + o.name + "::" +  className + "(*this);");
        cl.method(hdlMethod);
    }
}
void writeObjBaseMethods(Class& cl, const xml::Object& o) {
    Method insert("insert", "std::string");
    insert.protected_()
        .param(Variable("tables", "litesql::Record&"))
        .param(Variable("fieldRecs", "litesql::Records&"))
        .param(Variable("valueRecs", "litesql::Records&"))
        .body("tables.push_back(table__);")
        .body("litesql::Record fields;")
        .body("litesql::Record values;");

    if (o.parentObject)
        insert.body("fields.push_back(\"id_\");")
            .body("values.push_back(id);");

    for (size_t i = 0; i < o.fields.size(); i++) {
        const xml::Field& f = *o.fields[i];
        insert.body("fields.push_back(" + f.name + ".name()" + ");");
        insert.body("values.push_back(" + f.name + ");");
        insert.body(f.name + ".setModified(false);");
    }
    
    insert.body("fieldRecs.push_back(fields);")
        .body("valueRecs.push_back(values);");
    if (o.parentObject) {
        insert.body("return " + o.inherits 
                    + "::insert(tables, fieldRecs, valueRecs);");
    } else
        insert.body("return " + o.inherits
                    + "::insert(tables, fieldRecs, valueRecs, " 
                    + "sequence__);");


    Method create("create", "void");
    create.protected_();
    create.body("litesql::Record tables;")
        .body("litesql::Records fieldRecs;")
        .body("litesql::Records valueRecs;")
        .body("type = type__;")
        .body("std::string newID = insert(tables, fieldRecs, valueRecs);")
        .body("if (id == 0)")
        .body("    id = newID;");


    Method addUpdates("addUpdates", "void");
    addUpdates.protected_().virtual_()
        .param(Variable("updates", "Updates&"))
        .body("prepareUpdate(updates, table__);");
    for (size_t i = 0; i < o.fields.size(); i++) {
        const xml::Field& f = *o.fields[i];
        addUpdates.body("updateField(updates, table__, " + f.name + ");");
    }
    if (o.parentObject) 
        addUpdates.body(o.inherits + "::addUpdates(updates);");
    Method addIDUpdates("addIDUpdates", "void");    
    addIDUpdates.protected_().virtual_()
        .param(Variable("updates", "Updates&"));
    if (o.parentObject) {
        addIDUpdates
            .body("prepareUpdate(updates, table__);")
            .body("updateField(updates, table__, id);");
        if (o.parentObject->parentObject)
            addIDUpdates.body(o.inherits + "::addIDUpdates(updates);");
    }
    
    Method getFieldTypes("getFieldTypes", "void");
    getFieldTypes.static_().
        param(Variable("ftypes", "std::vector<litesql::FieldType>&"));
    if (o.parentObject) 
        getFieldTypes.body(o.parentObject->name + "::getFieldTypes(ftypes);");
    for (size_t i = 0; i < o.fields.size(); i++)
        getFieldTypes.body("ftypes.push_back(" + o.fields[i]->fieldTypeName + ");");
    
    Method update("update", "void");
    update.virtual_()
        .body("if (!inDatabase) {")
        .body("    create();")
        .body("    return;")
        .body("}")
        .body("Updates updates;")
        .body("addUpdates(updates);")
        .body("if (id != oldKey) {")
        .body("    if (!typeIsCorrect()) ")
        .body("        upcastCopy()->addIDUpdates(updates);");
    if (o.parentObject) 
        update
            .body("    else")
            .body("        addIDUpdates(updates);");
    
    update
        .body("}")
        .body("litesql::Persistent::update(updates);")
        .body("oldKey = id;");

    Method delRecord("delRecord", "void");
    delRecord.protected_().virtual_()
        .body("deleteFromTable(table__, id);");
    if (o.parentObject)
        delRecord.body(o.inherits + "::delRecord();");
    Method delRelations("delRelations", "void");
    delRelations.protected_().virtual_();
    for (map<xml::Relation*, vector<xml::Relate*> >::const_iterator i = 
             o.relations.begin(); i != o.relations.end(); i++) {
        const xml::Relation * rel = i->first;
        const vector<xml::Relate *> relates = i->second;
        Split params;
        for (size_t i2 = 0; i2 < relates.size(); i2++)
            params.push_back("(" + rel->getName() + "::" + relates[i2]->fieldTypeName
                             + " == id)");
        delRelations.body(rel->getName() + "::del(*db, " + params.join(" || ") + ");");
    }
        
    

    Method del("del", "void");
    del.virtual_()
        .body("if (typeIsCorrect() == false) {")
        .body("    std::auto_ptr<" + o.name + "> p(upcastCopy());")
        .body("    p->delRelations();")
        .body("    p->onDelete();")
        .body("    p->delRecord();")
        .body("} else {")
        .body("    onDelete();")
        .body("    delRecord();")
        .body("}")
        .body("inDatabase = false;");


    Method typeIsCorrect("typeIsCorrect", "bool");    
    typeIsCorrect.body("return type == type__;").virtual_();
    
    Method upcast("upcast", "std::auto_ptr<" + o.name + ">");
    Method upcastCopy("upcastCopy", "std::auto_ptr<" + o.name + ">");
    Split names;
    o.getChildrenNames(names);
    upcastCopy.body(o.name + "* np = NULL;");
    for (size_t i = 0; i < names.size(); i++) {
        upcast.body("if (type == " + names[i] + "::type__)")
            .body("    return auto_ptr<" + o.name + ">(new " + names[i] 
                  + "(select<" + names[i] 
                  + ">(*db, Id == id).one()));");
        upcastCopy.body("if (type == " + quote(names[i]) + ")")
            .body("    np = new " + names[i] + "(*db);");
    }
    for (size_t i = 0; i < o.fields.size(); i++) {
        upcastCopy.body("np->" + o.fields[i]->name + " = " 
                        + o.fields[i]->name + ";");
    }
    upcastCopy
        .body("np->inDatabase = inDatabase;")
        .body("if (!np)")
        .body("    np = new " + o.name + "(*this);")
        .body("return auto_ptr<" + o.name + ">(np);");
    upcast.body("return auto_ptr<" + o.name 
                + ">(new " + o.name + "(*this));");

    for (size_t i = 0; i < o.methods.size(); i++) {
        const xml::Method& m = *o.methods[i];
        Method mtd(m.name, m.returnType);
        for (size_t i2 = 0; i2 < m.params.size(); i2++) {
            const xml::Param& p = m.params[i2];
            mtd.param(Variable(p.name, p.type));
        
        }
        mtd.defineOnly().virtual_();
        cl.method(mtd);
    }
    
    cl.method(insert).method(create).method(addUpdates)
        .method(addIDUpdates).method(getFieldTypes).method(delRecord).method(delRelations)
         .method(update).method(del)
        .method(typeIsCorrect).method(upcast).method(upcastCopy);

}

void writeStaticRelData(Class& cl, const xml::Relation& r) {
    Variable table("table__", "const std::string", 
                   quote(r.getTable()));
    table.static_();
    cl.variable(table);
    Method initValues("initValues", "void");
    bool hasValues = false;
    initValues.static_();
    for (size_t i2 = 0; i2 < r.related.size(); i2++) {
        Variable ftype(r.related[i2]->fieldTypeName,
                       "const litesql::FieldType",
                       quote(r.related[i2]->fieldName)
                       + "," + quote("INTEGER") + "," + "table__");
        ftype.static_();
        cl.variable(ftype);
    }
    for (size_t i2 = 0; i2 < r.fields.size(); i2++) {
        const xml::Field& fld = *r.fields[i2];
        string data = quote(fld.name + "_") + "," +
                      quote(fld.getSQLType()) + "," +

                      "table__";
        if (!fld.values.empty()) {
            data += "," + fld.name + "_values"; 
            Variable values(fld.name + "_values", 
                            "std::vector < std::pair< std::string, std::string > >");
            values.static_().protected_();
            cl.variable(values);
            initValues.body(fld.name + "_values.clear();");
            hasValues = true;
            for (size_t i2 = 0; i2 < fld.values.size(); i2++) {
                const xml::Value& val = fld.values[i2];
                initValues.body(fld.name + "_values.push_back(make_pair("
                          + quote(val.name) + "," + quote(val.value) + "));");    
            }

        }
        string ftypeClass ="const litesql::FieldType";
        if (!fld.values.empty()) {
            ftypeClass = fld.fieldTypeName + "Type";
            Class ftypeCl(ftypeClass, "litesql::FieldType");
            Method cons(ftypeClass);
   	        ftypeClass = "const " + r.getName() + "::" + ftypeClass;
            cons.param(Variable("n", "const std::string&"))
                .param(Variable("t", "const std::string&"))
                .param(Variable("tbl", "const std::string&"))
                .param(Variable("vals", "const litesql::FieldType::Values&", "Values()"))
                .constructor("litesql::FieldType(n,t,tbl,vals)");
            
            ftypeCl.method(cons);
            for (size_t v = 0; v < fld.values.size(); v++) {
                const xml::Value& value = fld.values[v];
                string v;
                if (fld.getCPPType() == "std::string")
                    v = quote(value.value);
                else
                    v = value.value;
                Variable val(value.name, "const " + fld.getCPPType(), v);
                
                val.static_();
                ftypeCl.variable(val);
            }
            cl.class_(ftypeCl);
        }
        Variable ftype(fld.fieldTypeName, ftypeClass, data);

        ftype.static_();
        cl.variable(ftype);
    }
    if (hasValues)
        cl.method(initValues);
    Class rowcl("Row");
    Method rowcons("Row");
    rowcons.param(Variable("db", "const litesql::Database&"))
        .param(Variable("rec", "const litesql::Record&", "litesql::Record()"));
//        .constructor("litesql::Record(db, rec)");
    Split consParams;
    int fieldNum = r.related.size() + r.fields.size();
    rowcons.body("switch(rec.size()) {");
    for (int i = r.fields.size()-1; i >= 0; i--) {
        const xml::Field& fld = *r.fields[i];
        Variable fldvar(fld.name, "litesql::Field<" + fld.getCPPType() + ">");
        rowcl.variable(fldvar);        

        rowcons.body("case " + toString(fieldNum) + ":")
            .body("    " + fld.name + " = rec[" + toString(fieldNum-1) + "];");
        consParams.push_back(fld.name +
                             "(" + r.getName() + "::" + fld.fieldTypeName + ")");
        fieldNum--;
        
    }
    for (int i = r.related.size()-1; i >= 0; i--) {
        const xml::Relate& rel = *r.related[i];
        string fname = xml::decapitalize(rel.fieldTypeName);
        Variable fld(fname, "litesql::Field<int>");
        rowcl.variable(fld);
        rowcons.body("case " + toString(fieldNum) + ":")
            .body("    " + fname + " = rec[" 
                  + toString(fieldNum-1) + "];"); 

        consParams.push_back(fname 
                             + "(" + r.getName() 
                             + "::" + rel.fieldTypeName +")");
        fieldNum--;
    }
    rowcons.body("}");
    rowcons.constructor(consParams.join(", "));
    rowcl.method(rowcons);    
    cl.class_(rowcl);

}
void writeRelMethods(xml::Database& database,
		     Class& cl, xml::Relation& r) {
    Variable dbparam("db", "const litesql::Database&");
    Variable destExpr("expr", "const litesql::Expr&", 
                       "litesql::Expr()");
    Variable srcExpr("srcExpr", "const litesql::Expr&", 
                       "litesql::Expr()");
    Method link("link", "void");
    Method unlink("unlink", "void");
    Method del("del", "void");
    Method getRows("getRows", 
                   "litesql::DataSource<"+r.getName()+"::Row>");
    
    link.static_().param(dbparam);
    link.body("Record values;")
        .body("Split fields;");
    for (size_t i = 0; i < r.related.size(); i++) {
        xml::Relate& rel = *r.related[i];
        link.body("fields.push_back(" + rel.fieldTypeName + ".name());");
        link.body("values.push_back(o" + toString(i) + ".id);");
        rel.paramPos = i;
    }
    for (size_t i = 0; i < r.fields.size(); i++) {
        xml::Field& fld = *r.fields[i];
        
        link.body("fields.push_back(" + fld.fieldTypeName + ".name());");
        if (fld.getCPPType() != "std::string")
            link.body("values.push_back(toString(" + fld.name + "));");
        else
            link.body("values.push_back(" + fld.name + ");");
    }
    link.body("db.insert(table__, values, fields);");
    if (r.isUnidir()==false && r.related.size() == 2 && r.sameTypes() == 2) {
        link.body("fields.clear();")
            .body("values.clear();");
        for (size_t i = 0; i < r.related.size(); i++) {
            xml::Relate& rel = *r.related[i];
            link.body("fields.push_back(" + rel.fieldTypeName + ".name());");
            link.body("values.push_back(o" + toString(1-i) + ".id);");
        }
        for (size_t i = 0; i < r.fields.size(); i++) {
            xml::Field& fld = *r.fields[i];
        
            link.body("fields.push_back(" + fld.fieldTypeName + ".name());");
            if (fld.getCPPType() != "std::string")
                link.body("values.push_back(toString(" + fld.name + "));");
            else
                link.body("values.push_back(" + fld.name + ");");
        }
        link.body("db.insert(table__, values, fields);");
    }

    unlink.static_().param(dbparam);
    Split unlinks;
    for (size_t i = 0; i < r.related.size(); i++) {
        xml::Relate& rel = *r.related[i];
        unlinks.push_back(rel.fieldTypeName + " == o" 
                          + toString(i) + ".id");
    }
    for (size_t i = 0; i < r.fields.size(); i++) {
        xml::Field& fld = *r.fields[i];
        unlinks.push_back("(" + fld.fieldTypeName + " == " + fld.name + ")");
    }
    
    unlink.body("db.delete_(table__, (" + unlinks.join(" && ") + "));");
    if (r.isUnidir()==false && r.related.size() == 2 && r.sameTypes() == 2) {
        unlinks.clear();
        for (size_t i = 0; i < r.related.size(); i++) {
            xml::Relate& rel = *r.related[i];
            unlinks.push_back(rel.fieldTypeName + " == o" 
                              + toString(1-i) + ".id");
        }
        for (size_t i = 0; i < r.fields.size(); i++) {
            xml::Field& fld = *r.fields[i];
            unlinks.push_back("(" + fld.fieldTypeName + " == " + fld.name + ")");
        }
        unlink.body("db.delete_(table__, (" + unlinks.join(" && ") + "));");
    }

    del.static_().param(dbparam).param(destExpr); 
    del.body("db.delete_(table__, expr);");
    getRows.static_().param(dbparam).param(destExpr)
        .body("SelectQuery sel;");

    for (size_t i = 0; i < r.related.size(); i++) {
        xml::Relate& rel = *r.related[i];        
        getRows.body("sel.result(" + rel.fieldTypeName + ".fullName());");
    }
    for (size_t i = 0; i < r.fields.size(); i++) {
        xml::Field& fld = *r.fields[i];
        getRows.body("sel.result(" + fld.fieldTypeName + ".fullName());");
    }
    getRows.body("sel.source(table__);")
        .body("sel.where(expr);")
        .body("return DataSource<" + r.getName() + "::Row>(db, sel);");

    for (size_t i2 = 0; i2 < r.related.size(); i2++) {
        xml::Relate& rel = *r.related[i2];
        Variable obj("o" + toString(i2), "const " +  database.nspace + "::" +rel.objectName + "&");
        link.param(obj);
        unlink.param(obj);
    }
    for (size_t i2 = 0; i2 < r.fields.size(); i2++) {
        xml::Field& fld = *r.fields[i2];
        link.param(Variable(fld.name, fld.getCPPType(), 
                            fld.getQuotedDefaultValue()));
        unlink.param(Variable(fld.name,  fld.getCPPType()));
    }
    cl.method(link).method(unlink).method(del).method(getRows);
    if (r.sameTypes() == 1) {
        Method getTpl("get", "litesql::DataSource<T>");
        getTpl.static_().template_("class T").defineOnly()
            .param(dbparam).param(destExpr).param(srcExpr);
        cl.method(getTpl);
        for (size_t i2 = 0; i2 < r.related.size(); i2++) {
            xml::Relate& rel = *r.related[i2];
            Method get("get", "litesql::DataSource<" 
                       + database.nspace + "::" + rel.objectName + ">");
            rel.getMethodName = "get<" + rel.objectName + ">";
            get.static_().templateSpec("")
                .param(dbparam).param(destExpr).param(srcExpr)
                .body("SelectQuery sel;")
                .body("sel.source(table__);")
                .body("sel.result(" + rel.fieldTypeName + ".fullName());")
                .body("sel.where(srcExpr);")
                .body("return DataSource<" + database.nspace + "::" + rel.objectName 
                      + ">(db, "+database.nspace + "::" + rel.objectName+"::Id.in(sel) && expr);");
            cl.method(get);
        }
    } else {
        map<string, int> counter;
        for (size_t i2 = 0; i2 < r.related.size(); i2++) {
            string num;
            xml::Relate& rel = *r.related[i2];
            if (r.countTypes(rel.objectName) > 1) {
                if (counter.find(rel.objectName) == counter.end())
                    counter[rel.objectName] = 0;
                counter[rel.objectName]++;
                num = toString(counter[rel.objectName]);        
            }
            rel.getMethodName = "get" + rel.objectName + num;
            Method get(rel.getMethodName, 
                       "litesql::DataSource<" + database.nspace + "::" 
                        + rel.objectName + ">");

            get.static_()
                .param(dbparam).param(destExpr).param(srcExpr)
                .body("SelectQuery sel;")
                .body("sel.source(table__);")
                .body("sel.result(" + rel.fieldTypeName + ".fullName());")
                .body("sel.where(srcExpr);")
                .body("return DataSource<" + database.nspace + "::" 
                     + rel.objectName 
                      + ">(db, "+database.nspace + "::" + rel.objectName+"::Id.in(sel) && expr);");
            cl.method(get);
        }
    }


}





void getSchema(const xml::Database& db,
               const vector<xml::Object*>& objects,
               const vector<xml::Relation*>& relations,
               Method& mtd) {    
//    Records recs;
    Split rec;
    mtd.body("vector<Database::SchemaItem> res;");
    rec.push_back(quote("schema_"));
    rec.push_back(quote("table"));
    rec.push_back(quote("CREATE TABLE schema_ (name_ TEXT, type_ TEXT, sql_ TEXT);"));
//    recs.push_back(rec);
    mtd.body("res.push_back(Database::SchemaItem(" + rec.join(",") + "));");
    mtd.body("if (backend->supportsSequences()) {");
    for (size_t i = 0; i < db.sequences.size(); i++) {
        Split rec;
        rec.push_back(quote(db.sequences[i]->name));
        rec.push_back(quote("sequence"));
        rec.push_back(quote(db.sequences[i]->getSQL()));
        mtd.body("    res.push_back(Database::SchemaItem(" + rec.join(",") + "));");
    }
    mtd.body("}");
    for (size_t i = 0; i < db.tables.size(); i++) {
        Split rec;
        rec.push_back(quote(db.tables[i]->name));
        rec.push_back(quote("table"));
        rec.push_back(quote(db.tables[i]->getSQL("\" + backend->getRowIDType() + \"")));
        mtd.body("res.push_back(Database::SchemaItem(" + rec.join(",") + "));");
    }
    for (size_t i = 0; i < db.indices.size(); i++) {
        Split rec;
        rec.push_back(quote(db.indices[i]->name));
        rec.push_back(quote("index"));
        rec.push_back(quote(db.indices[i]->getSQL()));
        mtd.body("res.push_back(Database::SchemaItem(" + rec.join(",") + "));");        
    }
    report(toString(db.tables.size()) + " tables\n");
    report(toString(db.sequences.size()) + " sequences\n");
    report(toString(db.indices.size()) + " indices\n");
}
void writeDatabaseClass(FILE* hpp, FILE* cpp,
                        xml::Database& dbInfo,
                        vector<xml::Object*>& objects,
                        vector<xml::Relation*>& relations) {
    gen::Class db(dbInfo.name, "litesql::Database");    
    gen::Method cons(dbInfo.name);
    
    cons.param(Variable("backendType", "std::string"))
        .param(Variable("connInfo", "std::string"))
        .constructor("litesql::Database(backendType, connInfo)");
    cons.body("initialize();");
    db.method(cons);
    gen::Method getSchemaMtd("getSchema", "std::vector<litesql::Database::SchemaItem>");
    getSchemaMtd.virtual_().protected_().const_();

    getSchema(dbInfo, objects, relations, getSchemaMtd);
/*    for (Records::iterator i = schema.begin(); i != schema.end(); i++) {
        if ((*i)[1] == quote("sequence"))
            getSchemaMtd.body("if (backend->supportsSequences())");
        getSchemaMtd.body("res.push_back(Database::SchemaItem(" 
                       + (*i)[0] + "," 
                       + (*i)[1] + ","
                       + (*i)[2] + "));");
    } */
    
    getSchemaMtd.body("return res;");

    db.method(getSchemaMtd);
    Method init("initialize", "void");
    init.body("static bool initialized = false;")
        .body("if (initialized)")
        .body("    return;")
        .body("initialized = true;");
    
    for (size_t i = 0; i < objects.size(); i++) {
        xml::Object& o = *objects[i];
        for (size_t i2 = 0; i2 < o.fields.size(); i2++)
            if (!o.fields[i2]->values.empty()) {
                init.body(o.name + "::initValues();");
                break;
            }
    }
    for (size_t i = 0; i < relations.size(); i++) {
        xml::Relation& r = *relations[i];
        for (size_t i2 = 0; i2 < r.fields.size(); i2++)
            if (!r.fields[i2]->values.empty()) {
                init.body(r.getName() + "::initValues();");
                break;
            }
    }

    init.protected_().static_();
    db.method(init);
    db.write(hpp, cpp);
}
void writeCPPClasses(xml::Database& db,
                     vector<xml::Object*>& objects,
                     vector<xml::Relation*>& relations) {
    sanityCheck(db, objects, relations);
    bool hasNamespace = false;
    
    FILE *hpp, *cpp;
    
    string hppName = toLower(db.name) + ".hpp";
    hpp = fopen(hppName.c_str(), "w");
    if (!hpp) {
        string msg = "could not open file : " + hppName;
        perror(msg.c_str());
        return;
    }
    
    string cppName = toLower(db.name) + ".cpp";
    cpp = fopen(cppName.c_str(), "w");
    if (!cpp) {
        string msg = "could not open file : " + cppName;
        perror(msg.c_str());
        return;
    }
    string defName = "_" + toLower(db.name) + "_hpp_";
    fprintf(hpp, "#ifndef %s\n", defName.c_str());
    fprintf(hpp, "#define %s\n", defName.c_str());
    fprintf(hpp, "#include \"litesql.hpp\"\n");
    if (!db.include.empty()) 
        fprintf(hpp, "#include \"%s\"\n", db.include.c_str());
    fprintf(cpp, "#include \"%s\"\n", hppName.c_str());
    

    if (!db.nspace.empty()) {
        fprintf(hpp, "namespace %s {\n", db.nspace.c_str());
        fprintf(cpp, "namespace %s {\n", db.nspace.c_str());
        hasNamespace = true;
    } else
        hasNamespace = false;
    fprintf(cpp, "using namespace litesql;\n");

    report("writing prototypes for Persistent classes\n"); 
    for (size_t i = 0; i < objects.size(); i++) 
        fprintf(hpp, "class %s;\n", objects[i]->name.c_str());

    report("writing relations\n");
    for (size_t i = 0; i < relations.size(); i++) {
        xml::Relation & o = *relations[i];
        Class cl(o.getName());
        writeStaticRelData(cl, o);
        writeRelMethods(db, cl, o);
        
        cl.write(hpp, cpp);
    }
    report("writing persistent objects\n");
    for (size_t i = 0; i < objects.size(); i++) {
        xml::Object & o = *objects[i];
        Class cl(o.name, o.inherits);
        writeStaticObjData(cl, o);
        writeObjFields(cl, o);       
        writeObjConstructors(cl, o);
        writeObjRelationHandles(cl, o);
        writeObjBaseMethods(cl, o);
        cl.write(hpp, cpp);

        // Object -> string method (not associated to class)

        gen::Method strMtd("operator<<", "std::ostream &");
        strMtd.param(Variable("os", "std::ostream&"))
            .param(Variable("o", o.name));
        vector<xml::Field*> flds;
        o.getAllFields(flds);

        strMtd.body("os << \"-------------------------------------\" << std::endl;");
        for (size_t i2 = 0; i2 < flds.size(); i2++) {
            xml::Field& fld = *flds[i2];
            strMtd.body("os << o." + fld.name + ".name() << \" = \" << o." 
                        + fld.name + " << std::endl;");
        }
        strMtd.body("os << \"-------------------------------------\" << std::endl;");
        strMtd.body("return os;");
        
        strMtd.write(hpp, cpp, "", 0);
    }
    report("writing database class\n");
    writeDatabaseClass(hpp, cpp, db, objects, relations);
    if (hasNamespace) {
        fprintf(hpp, "}\n");
        fprintf(cpp, "}\n");
    }

    fprintf(hpp, "#endif\n");

    fclose(hpp);
    fclose(cpp);
}

