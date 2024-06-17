<?php
class Unit {
    public $char;

    public $horizontalPos;
    public $verticalPos;

    public static $charSet = array('A', 'B', 'C', 'D', 'E', 'F');

    public function __construct($h, $v) {
        $this->horizontalPos = $h;
        $this->verticalPos = $v;
        $this->char = Unit::$charSet[array_rand(Unit::$charSet)];
    }

    public function getChar() {
        return $this->char;
    }

    public function getPosition() {
        return [$this->horizontalPos, $this->verticalPos];
    }

    public function setPosition($h, $v) {
        $this->horizontalPos = $h;
        $this->verticalPos = $v;
    }

    public function getHorizontalPos() {
        return $this->horizontalPos;
    }

    public function setHorizontalPos($h) {
        $this->horizontalPos = $h;
    }

    public function getVerticalPos() {
        return $this->verticalPos;
    }

    public function setVerticalPos($v) {
        $this->verticalPos = $v;
    }

    public function getCoords() {
        return [$horizontalPos, $verticalPos];
    }

    public function incrHorizontalPos() {
        $this->horizontalPos = $this->horizontalPos + 1;
    }

    public function decrHorizontalPos() {
        if ($this->horizontalPos > 0) {
            $this->horizontalPos = $this->horizontalPos - 1;
        }
    }

    public function incrVerticalPos() {
        $this->verticalPos = $this->verticalPos + 1;
    }
    
    public function decrVerticalPos() {
        if ($this->verticalPos > 0) {
            $this->verticalPos = $this->verticalPos - 1;
        }
    }

    public function __toString() {
        return $this->char;
    }
}