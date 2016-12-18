package ast;

import type.TypeRef;

import java.util.List;

/**
 * Created by sulvto on 16-12-8.
 */
public class StructNode extends CompositeTypeDefinition {

    public StructNode(Location location, TypeRef ref, String name, List<Slot> members) {
        super(location, ref, name, members);
    }

}
