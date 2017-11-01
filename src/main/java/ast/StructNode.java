package ast;

import lexer.Token;
import type.TypeRef;

import java.util.List;

/**
 * Created by sulvto on 16-12-8.
 */
public class StructNode extends CompositeTypeDefinition {

    public StructNode(Token token, TypeRef ref, String name, List<Slot> members) {
        super(token, ref, name, members);
    }

}
