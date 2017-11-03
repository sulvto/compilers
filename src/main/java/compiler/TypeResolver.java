package compiler;

import ast.*;
import entity.*;
import type.CompositeType;
import type.Type;
import type.TypeTable;
import util.ErrorHandler;

import java.util.List;

/**
 * Created by sulvto on 17-11-2.
 */
public class TypeResolver extends Visitor implements DeclarationVisitor<Void>, EntityVisitor<Void> {
    private final TypeTable typeTable;
    private final ErrorHandler errorHandler;

    public TypeResolver(TypeTable typeTable, ErrorHandler errorHandler) {
        this.errorHandler = errorHandler;
        this.typeTable = typeTable;
    }

    public void resolve(AST ast) {
        defineTypes(ast.types());
        ast.types().forEach(typeDefinition -> typeDefinition.accept(this));
        ast.entitys().forEach(typeDefinition -> typeDefinition.accept(this));
    }

    private void defineTypes(List<TypeDefinition> deftypes) {
        deftypes.forEach(typeDefinition -> {
            if (typeTable.isDefined(typeDefinition.typeRef())) {
                error(typeDefinition, "duplicated type definition: " + typeDefinition.typeRef());
            } else {
                typeTable.put(typeDefinition.typeRef(), typeDefinition.definingType());
            }
        });

    }

    @Override
    public Void visit(DefinedVariable variable) {
        bindType(variable.typeNode());
        if (variable.hasInitializer()) {
            visitExpr(variable.initializer());
        }
        return null;
    }

    @Override
    public Void visit(UndefinedVariable variable) {
        bindType(variable.typeNode());
        return null;
    }

    @Override
    public Void visit(DefinedFunction function) {
        resolveFunctionHeader(function);
        visitStmt(function.getBody());
        return null;
    }

    @Override
    public Void visit(UndefinedFunction function) {
        resolveFunctionHeader(function);
        return null;
    }

    @Override
    public Void visit(Constant constant) {
        bindType(constant.typeNode());
        visitExpr(constant.value());
        return null;
    }

    private void resolveFunctionHeader(Function function) {
        bindType(function.typeNode());

        function.parameters().forEach(parameter -> {
            Type t = typeTable.getParamType(parameter.typeNode().typeRef());
            parameter.typeNode().setType(t);
        });
    }

    private void bindType(TypeNode typeNode) {
        if (typeNode.isResolved()) return;
        typeNode.setType(typeTable.get(typeNode.typeRef()));
    }


    @Override
    public Void visit(StructNode structNode) {
        resolveCompositeType(structNode);
        return null;
    }

    @Override
    public Void visit(UnionNode unionNode) {
        resolveCompositeType(unionNode);
        return null;
    }

    public void resolveCompositeType(CompositeTypeDefinition def) {
        CompositeType compositeType = (CompositeType) typeTable.get(def.typeNode().typeRef());
        if (compositeType == null) {
            throw new Error("cannot intern struct/union");
        }

        compositeType.members()
    }

    @Override
    public Void visit(TypedefNode typedefNode) {
        bindType(typedefNode.typeNode());
        bindType(typedefNode.realTypeNode());
        return null;
    }

    @Override
    public Void visit(BlockNode node) {
        for (DefinedVariable variable : node.variables()) {
            variable.accept(this);
        }
        visitStmts(node.stmts());
        return null;
    }


    @Override
    public Void visit(IntegerLiteralNode node) {
        bindType(node.typeNode());
        return null;
    }

    @Override
    public Void visit(StringLiteralNode node) {
        bindType(node.typeNode());
        return null;
    }


}


