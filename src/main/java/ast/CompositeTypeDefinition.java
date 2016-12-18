package ast;

import type.TypeRef;

import java.util.List;

/**
 * Created by sulvto on 16-12-8.
 */
public abstract class CompositeTypeDefinition extends TypeDefinition {
    private List<Slot> members;

    public CompositeTypeDefinition(Location location, TypeRef ref, String name,List<Slot> members) {
        super(location, ref, name);
        this.members = members;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("name", name);
        dumper.printMember("members", members);
    }
}
