package ast;

import type.TypeRef;
import type.UserTypeRef;

/**
 * Created by sulvto on 16-12-13.
 */
public class TypedefNode extends TypeDefinition {
    private TypeNode real;

    public TypedefNode(Location location, TypeRef real, String name) {
        super(location, new UserTypeRef(name), name);
        this.real = new TypeNode(real);
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("name", name);
        dumper.printMember("typeNode", real);
    }
}
