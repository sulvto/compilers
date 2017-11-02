package compiler;

import ast.AST;
import ast.DeclarationVisitor;
import ast.TypeDefinition;
import ast.TypeNode;
import entity.DefinedVariable;
import entity.EntityVisitor;
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

    private void bindType(TypeNode typeNode) {
        if (typeNode.isResolved()) return;
        typeNode.setType(typeTable.get(typeNode.typeRef()));
    }
}
