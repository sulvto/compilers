package assembler;

import java.util.Vector;

/**
 * Created by sulvto on 17-10-29.
 */
public class LabelSymbol {
    String name;
    int address;

    // Is this ref`d before def`d.
    boolean isForwardRef = false;
    boolean isDefined = true;
    // List of operands in memory we need to update after seeing def
    Vector<Integer> forwardReference = null;

    public LabelSymbol(String name) {
        this.name = name;
    }

    public LabelSymbol(String name, int address) {
        this.name = name;
        this.address = address;
    }

    public LabelSymbol(String name, int address, boolean forward) {
        this.name = name;
        this.isForwardRef = forward;
        if (forward) {
            addForwardReference(address);
        } else {
            this.address = address;
        }
    }

    public void addForwardReference(int address) {
        if (forwardReference == null) {
            forwardReference = new Vector<>();
        }
        forwardReference.addElement(new Integer(address));
    }


    @Override
    public String toString() {
        String refs = "";
        if (forwardReference != null) {
            refs = "[refs=" + forwardReference.toString() + "]";
        }
        return name + "@" + address + refs;
    }

    public void resolveForwardReferences(byte[] code) {
        isForwardRef = true;
        this.forwardReference.forEach(addrToPatch -> BytecodeAssembler.writeInt(code, addrToPatch, address));
    }
}
