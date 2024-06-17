<?php
class Group {
    // holds individual units
    private $slotA;
    private $slotB;

    // what position are we in?  (for rotation)
    private $pos;

    // horizontal or vertical
    private $orientation;

    public static $orientationHorizontal = "HORE";
    public static $orientationVertical = "VERTI";

    public function __construct($A = null, $B = null, $orientation = null) {
        if (is_null($A)) {
            $A = new Unit();
        }

        if (is_null($B)) {
            $B = new Unit();
        }

        $this->pos = 0;

        $this->slotA = $A;
        $this->slotB = $B;

        // randomize the orientation
        if (is_null($orientation)) {
            if (rand(0, 1)) {
                $this->orientation = Group::$orientationHorizontal;
            } else {
                $this->orientation = Group::$orientationVertical;
            }
        } else {
            $this->orientation = $orientation;
        }
    }

    public function getPos() {
        return $this->pos;
    }

    public function setPos($set = false) {
        $pos = $this->pos + 1;

        if ($set) {
            $this->pos++;
        }

        if ($pos > 3) {
            $pos = 0;
        }

        if ($this->pos > 3) {
            $this->pos = 0;
        }

        $p = $this->slotB->getPosition();
        $h1 = $p[0];
        $w1 = $p[1];

        $p = $this->slotA->getPosition();
        $h2 = $p[0];
        $w2 = $p[1];
            
        if ($pos == 0) {
            // 1-2
            echo "pos 0\n";
            $w1 = $w1 - 1;
 
            if ($set) {
                $this->orientation = Group::$orientationHorizontal;
            }
        } else if ($pos == 1) {
            // 1
            // 2
            echo "pos 1\n";
            $h1 = $h1 - 1;

            if ($set) {
                $this->orientation = Group::$orientationVertical;
            }
        } else if ($pos == 2) {
            // 2-1
            echo "pos 2\n";
            $w1 = $w1 + 1;

            if ($set) {
                $this->orientation = Group::$orientationHorizontal;
            }
        } else if ($pos == 3) {
            // 2
            // 1
            echo "pos 3\n";
            $h1 = $h1 + 1;

            if ($set) {
                $this->orientation = Group::$orientationVertical;
            }
        }

        if ($set) {
            $this->slotA->setPosition($h1, $w1);
        }

        return array([$h1, $w1], [$h2, $w2]); 
    }

    public function incrPos() {
        $this->pos++;

        if ($this->pos > 3) {
            $this->pos = 0;
        }

        $this->setPos();
    }
    public function getSlotA() {
        return $this->slotA;
    }

    public function setSlotA($a) {
        $this->slotA = $a;
    }

    public function getSlotB() {
        return $this->slotB;
    }

    public function setSlotB($b) {
        $this->slotB = $b;
    }

    public function getOrientation() {
        return $this->orientation;
    }

    public function setOrientation($o) {
        $this->orientation = $o;
    }

    public function incrVertical() {
        $this->slotA->incrVerticalPos();
        $this->slotB->incrVerticalPos();
    }

    public function decrVertical() {
        $this->slotA->decrVerticalPos();
        $this->slotB->decrVerticalPos();
    }

    public function incrHorizontal() {
        $this->slotA->incrHorizontalPos();
        $this->slotB->incrHorizontalPos();
    }

    public function decrHorizontal() {
        $this->slotA->decrHorizontalPos();
        $this->slotB->decrHorizontalPos();
    }

    public function inPosition($h, $v) {
        $h1 = $this->slotA->getHorizontalPos();
        $v1 = $this->slotA->getVerticalPos();

        $h2 = $this->slotB->getHorizontalPos();
        $v2 = $this->slotB->getVerticalPos();

        if (($h == $h1 && $v == $v1) || ($h == $h2 && $v == $v2)) {
            return true;
        }

        return false;
    }

    /**
     * returns a 2 element array containing the [x, y] for each unit
     */
    public function getPosition() {
        $h1 = $this->slotA->getHorizontalPos();
        $v1 = $this->slotA->getVerticalPos();

        $h2 = $this->slotB->getHorizontalPos();
        $v2 = $this->slotB->getVerticalPos();

        return [[$h1, $v1], [$h2, $v2]];
    }

    /**
     * like getPosition but instead of just the coords,
     * we return the characters FOR those coords
     */
    public function getPositionChars() {
        $h1 = $this->slotA->getHorizontalPos();
        $v1 = $this->slotA->getVerticalPos();

        $h2 = $this->slotB->getHorizontalPos();
        $v2 = $this->slotB->getVerticalPos();

        $charA = $this->slotA->getChar();
        $charB = $this->slotB->getChar();

        if (!isset($ret[$charA])) {
            $ret[$charA] = array();
        }

        if (!isset($ret[$charB])) {
            $ret[$charB] = array();
        }

        $ret[$charA][] = [$h1, $v1];
        $ret[$charB][] = [$h2, $v2];

        return $ret;
    }

    public function setPosition($h1, $v1, $h2, $v2) {
        $this->slotA->setHorizontalPos($h1);
        $this->slotB->setHorizontalPos($h2);
        $this->slotA->setVerticalPos($v1);
        $this->slotB->setVerticalPos($v2);
    }

    public function swap() {
        echo "swapping\n";
        $a = $this->getSlotA();
        $h1 = $a->getHorizontalPos();
        $v1 = $a->getVerticalPos();

        $b = $this->getSlotB();
        $h2 = $b->getHorizontalPos();
        $v2 = $b->getVerticalPos();

        $a->setHorizontalPos($h2);
        $a->setVerticalPos($v2);
        $b->setHorizontalPos($h1);
        $b->setVerticalPos($v1);
        
        $this->setSlotA($b);
        $this->setSlotB($a);
    }

    public function rotate() {
        $aPos = $this->slotA->getPosition();
        $bPos = $this->slotB->getPosition();

        $h1 = $aPos[0];
        $v1 = $aPos[1];

        $h2 = $bPos[0];
        $v2 = $bPos[1];

        if ($this->orientation == Group::$orientationHorizontal) {
            // is slot A or slot B on the left?
            if ($v1 > $v2) {
                // slot A;
                $this->slotA->setPosition($h1 - 1, $v1 - 1);
            } else {
                // slot B;
                $this->slotB->setPosition($h2 - 1, $v2 - 1);
            }

            $this->orientation = Group::$orientationVertical;
        } else if ($this->orientation == Group::$orientationVertical) {
            // is slot A on the top, or the bottom?
            if ($h1 < $h2) {
                $this->slotA->setPosition($h1 + 1, $v1 + 1);
            } else {
                $this->slotB->setPosition($h2 + 1, $v2 + 1);
            }

            $this->orientation = Group::$orientationHorizontal;
        } else {
            echo "error.\n";
            die();
        }
    }
}