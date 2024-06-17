<?php
class Game {
    private $term;

    private $width;
    private $height;

    private $grid;
    private $groups;
    private $activeGroup;

    private $ticks;
    private $moveTicks;

    private $score;

    private $permanentGroupCount;

    public function __construct() {
        $this->term = `stty -g`;
		system("stty -icanon -echo");

        $this->init();

        stream_set_blocking(STDIN, false); // Do not wait

        $this->gameLoop();
    }

    public function init() {
        // start the score at 0
        $this->score = 0;

        // initialize the groupcount
        $this->permanentGroupCount = 1;

        // initialize tickers
        $this->ticks = 0;
        $this->moveTicks = 10000;

        // use the default height and width
        $this->height = Config::$defaultHeight;
        $this->width = Config::$defaultWidth;

        // start with no groups
        $this->activeGroup = null;

        $this->groups = array();

        // intialize the grid
        // the grid holds the units no longer in play
        $this->grid = array();

        for ($x = 0; $x < $this->height; $x++) {
            for ($y = 0; $y < $this->width; $y++) {
                $this->grid[$x][$y] = null;
            }
        }

        // add some junk to the end of the grid
        $this->grid[0][$this->width - 1] = new Unit(0, $this->width - 1);

        // add a group
        $this->addGroup();
    }
    
    public function gameLoop() {
        echo $this;

        while (1) {
            $this->ticks++;
    
            $redraw = false;
            
            if ($this->ticks >= $this->moveTicks) {
                $this->ticks = 0;

                // move the pieces
                $this->moveUnits();
                $this->moveGroups();
                $this->removeMatches();

                $redraw = true;
            } 
            
            $group = null;

            // get the coordinates for the active group
            if (!is_null($this->activeGroup)) {
                $group = $this->groups[$this->activeGroup];
                $coords = $group->getPosition();

                $curH1 = $coords[0][0];
                $curV1 = $coords[0][1];
                $curH2 = $coords[1][0];
                $curV2 = $coords[1][1];
            }

            // wait for user input and process
            $c = Config::waitForInput();

            switch ($c) {
                case 'a':
                    $this->addGroup();

                    $redraw = true;

                    break;
                case 'g':
                    print_r($this->groups);
                    break;
                case 'q':
                    $this->gameOver();
                    
                    break;
                case 'r':
                    if (!is_null($this->activeGroup)) {
                        $redraw = $this->rotateGroup($this->activeGroup);
                    }

                    break;
                case Keypress::upArrow():
                case Keypress::doubleUpArrow():
                    if (!is_null($group)) {
                        if ($curH1 > 0 && $curH2 > 0) {
                            if ($this->checkMove($curH1 - 1, $curV1, $curH2 - 1, $curV2, $this->activeGroup)) {
                                // move checks out!
                                $group->decrHorizontal();

                                $redraw = true;
                            } else {
                                echo "we have a collision on an up move\n";
                                
                                $this->convertGroupToUnits($this->activeGroup);

                                if (count($this->groups) < $this->permanentGroupCount) {
                                    $this->addGroup();
                                }

                                $redraw = true;
                            }
                        }
                    }

                    break;
                case Keypress::downArrow():
                case Keypress::doubleDownArrow():
                    if (!is_null($group)) {
                        $height = $this->height - 1;

                        if ($curH1 < $height && $curH2 < $height) {
                            if ($this->checkMove($curH1 + 1, $curV1, $curH2 + 1, $curV2, $this->activeGroup)) {
                                // move checks out!
                                $group->incrHorizontal();
                                $redraw = true;
                            } else {
                                echo "we have a collision on a down move\n";

                                $this->convertGroupToUnits($this->activeGroup);

                                if (count($this->groups) < $this->permanentGroupCount) {
                                    $this->addGroup();
                                }

                                $redraw = true;
                            }
                        } 
                    }

                    break;
                case Keypress::leftArrow():
                case Keypress::doubleLeftArrow():
                    /*
                    if (!is_null($group)) {
                        if ($curV1 > 0 && $curV2 > 0) {
                            if ($this->checkMove($curH1, $curV1 - 1, $curH2, $curV2 - 1, $this->activeGroup)) {
                                // move checks out
                                $group->decrVertical();

                                $redraw = true;
                            } else {
                                echo "we have a collision on a left move\n";

                                $this->convertGroupToUnits($this->activeGroup);

                                if (count($this->groups) < $this->permanentGroupCount) {
                                    $this->addGroup();
                                }

                                $redraw = true;
                            }
                        }
                    }
                    */
                    break;
                case Keypress::rightArrow():
                case Keypress::doubleRightArrow():
                    if (!is_null($group)) {
                        $width = $this->width - 1;

                        if ($curV1 < $width && $curV2 < $width) {
                            if ($this->checkMove($curH1, $curV1 + 1, $curH2, $curV2 + 1, $this->activeGroup)) {
                                // move checks out
                                $group->incrVertical();

                                $redraw = true; 
                            } else {
                                echo "We have a collision on a right move\n";
                                $this->convertGroupToUnits($this->activeGroup);

                                if (count($this->groups) < $this->permanentGroupCount) {
                                    $this->addGroup();
                                }

                                $redraw = true;
                            }
                        } else {
                            echo "We ran into the bottom!\n";
                            $this->convertGroupToUnits($this->activeGroup);

                            if (count($this->groups) < $this->permanentGroupCount) {
                                $this->addGroup();
                            }

                            $redraw = true;
                        }
                    }

                    break;
                case Keypress::shiftLeftArrow():
                    if ($this->activeGroup > 0) {
                        $this->activeGroup--;
                    }

                    $redraw = true;

                    break;
                case Keypress::shiftRightArrow():
                    $max = count($this->groups) - 1;

                    if ($this->activeGroup < $max) {
                        $this->activeGroup++;
                    }

                    $redraw = true;

                    break;
                case Keypress::spaceBar():
                    if (!is_null($this->activeGroup)) {
                        $this->groups[$this->activeGroup]->swap();

                        $redraw = true;
                    }

                    break;
                case Keypress::enter():
                    if (!is_null($this->activeGroup)) {
                        $this->slideRight($this->activeGroup);

                        $redraw = true;
                    }

                    break;
                default:
                    // unhandled keypress
                    if ($c != '') {
                        echo "unhandled keypress: ";
                        print_r(Config::strToChr($c));
                    }
            }

            if ($redraw) {
                echo $this;
            }
        }

        $this->gameOver();
    }

    public function gameOver() {
        system("stty " . $this->term);

        exit;
    }

    private function slideRight($gid = null) {
        if (!is_null($gid)) {
            $g = $this->groups[$gid];
            $coords = $g->getPosition();
            $orientation = $g->getOrientation();

            $h1 = $coords[0][0];
            $v1 = $coords[0][1];
            $h2 = $coords[1][0];
            $v2 = $coords[1][1];

            $maxWidth = $this->width - 1;

            // keep sliding down vertically 1 space until we're block
            do {
                $v1 = $v1 + 1;
                $v2 = $v2 + 1;

                echo "moving vertically by 1 ($h1, $v1) ($h2, $v2)\n";
            } while ($this->checkMove($h1, $v1, $h2, $v2, $gid) && ($v1 <= $maxWidth && $v2 <= $maxWidth));

            // set the new coordinates if we've moved
            if ($v1 - 1 > $coords[0][1] && $v2 - 1 > $coords[1][1]) {
                $this->groups[$gid]->setPosition($h1, $v1 - 1, $h2, $v2 - 1);
            }

            // break up the group
            $this->convertGroupToUnits($gid);

            if (count($this->groups) < $this->permanentGroupCount) {
                $this->addGroup();
            }
        }
    }

    private function rotateGroup($gid = null) {
        if (!is_null($gid)) {
            $pos = $this->groups[$gid]->setPos();
            $h1 = $pos[0][0];
            $v1 = $pos[0][1];
            $h2 = $pos[1][0];
            $v2 = $pos[1][1];

            if ($this->checkMove($h1, $v1, $h2, $v2, $gid)) {
                $this->groups[$gid]->setPos(true);

                echo "rotating group #$gid ($h1, $v1) ($h2, $v2)\n";

                return true;
            }
        }

        return false;
    }

    private function moveUnits() {
        for ($x = 0; $x < $this->height; $x++) {
            for ($y = $this->width - 2; $y > 0; $y--) {
                // is the space beside this one free?
                if (is_null($this->grid[$x][$y + 1])) {
                    $this->grid[$x][$y + 1] = $this->grid[$x][$y];
                    $this->grid[$x][$y] = null;
                }
            }
        }
    }

    private function moveGroups() {
        // all of the active groups get moved 
        for ($g = 0; $g < count($this->groups); $g++) {
            echo "Moving group g:$g\n";
            $group = $this->groups[$g];

            // get current position
            $coords = $group->getPosition();

            // calculate new position (we are moving vertical)
            $h1 = $coords[0][0];
            $v1 = $coords[0][1];

            $h2 = $coords[1][0];
            $v2 = $coords[1][1];

            echo "Checking coords ($h1, " . ($v1 + 1) . ") ($h2, " . ($v2 + 1) . ")\n";

            // see if position is free
            $width = $this->width - 1;

            if ($v1 < $width && $v2 < $width && $this->checkMove($h1, $v1 + 1, $h2, $v2 + 1, $g)) {
                // adjust the group units position
                echo "Position is free.  Moving group.\n";
                $group->setPosition($h1, $v1 + 1, $h2, $v2 + 1);
            } else {
                // we have a collision with something
                echo "Collision on group #$g\n";

                // if we are at bottom, or ON another square convert back to units
                $this->convertGroupToUnits($g);

                if (count($this->groups) < $this->permanentGroupCount) {
                    $this->addGroup();
                }
            }
        }
    }

    private function convertGroupToUnits($gid) {
        echo "Converting group #$gid to units\n";
        $group = $this->groups[$gid];

        // remove the group
        array_splice($this->groups, $gid, 1);

        $slotA = $group->getSlotA();
        $h1 = $slotA->getHorizontalPos();
        $v1 = $slotA->getVerticalPos();

        $this->grid[$h1][$v1] = $slotA;

        $slotB = $group->getSlotB();
        $h2 = $slotB->getHorizontalPos();
        $v2 = $slotB->getVerticalPos();

        $this->grid[$h2][$v2] = $slotB;

        if ($this->activeGroup > count($this->groups) - 1) {
            $this->activeGroup = count($this->groups) - 1;

            if ($this->activeGroup < 0) {
                $this->activeGroup = null;
            }
        }
    }

    /**
     * get an array of chars in their positions
     */
    private function gridToArray() {
        $ret = array();

        for ($x = 0; $x < $this->height; $x++) {
            if (!isset($ret[$x])) {
                $ret[$x] = array();
            }

            for ($y = 0; $y < $this->width; $y++) {
                if (isset($this->grid[$x][$y]) && !is_null($this->grid[$x][$y])) {
                    $ret[$x][$y] = $this->grid[$x][$y]->getChar();
                } else {
                    $ret[$x][$y] = null;
                }
            }
        }

        return $ret;
    }

    private function gridAndGroupsToArray() {
        $ret = array();

        for ($x = 0; $x < $this->height; $x++) {
            if (!isset($ret[$x])) {
                $ret[$x] = array();
            }

            for ($y = 0; $y < $this->width; $y++) {
                if ($this->findInGroups($x, $y)) {
                    $ret[$x][$y] = "X";
                } else if (isset($this->grid[$x][$y]) && !is_null($this->grid[$x][$y])) {
                    // if we're not part of a group, check if we're part of the grid
                    $ret[$x][$y] = "X";
                } else {
                    $ret[$x][$y] = null;
                }
            }
        }

        return $ret;
    }

    private function findInGroups($h, $v) {
        for ($gid = 0; $gid < count($this->groups); $gid++) {
            $pos = $this->groups[$gid]->getPosition();

            if (in_array([$h, $v], $pos)) {
                return $gid;
            }
        }

        return false;
    }

    private function removeMatches() {
        $array = $this->gridToArray();

        $matches = $this->findMatches($array);

        if (count($matches)) {
            echo "We have some matches!\n";
        
            // if any of the matches are part of a group, split that group up

            // remove any matches in the grid
            foreach ($matches as $tmp) {
                foreach ($tmp as $m) {
                    $h = $m[0];
                    $v = $m[1];
            
                    $gid = $this->findInGroups($h, $v);

                    // this coordinate is part of a grouping
                    if ($gid !== false) {
                        // founnd
                        $group = $this->groups[$gid];

                        $a = $group->getSlotA();
                        $aPos = $a->getPosition();

                        $b = $group->getSlotB();
                        $bPos = $b->getPosition();

                        // break it up
                        $this->convertGroupToUnits($gid);

                        if (count($this->groups) < $this->permanentGroupCount) {
                            $this->addGroup();
                        }
                    }

                    // nuke the offending grid unit
                    $this->score++;
                    $this->grid[$h][$v] = null;
                }
            }
        }
    }

    private function checkDirection($board, $x, $y, $dx, $dy) {
        $piece = $board[$x][$y];

        if (is_null($piece)) {
            return false;
        }

        for ($i = 1; $i < 3; $i++) {
            $nx = $x + $i * $dx;
            $ny = $y + $i * $dy;
        
            if ($nx < 0 || $nx >= count($board) || $ny < 0 || $ny >= count($board[0]) || $board[$nx][$ny] != $piece) {
                return false;
            }
        }

        return true;
    }

    function findMatches($board) {
        $rows = count($board);
        $cols = count($board[0]);
        $matches = array();
        
        for ($x = 0; $x < $rows; $x++) {
            for ($y = 0; $y < $cols; $y++) {
                if (!is_null($board[$x][$y])) {
                    // Check horizontal (right)
                    if ($y <= $cols - 3 && $this->checkDirection($board, $x, $y, 0, 1)) {
                        $match = array();
                        
                        for ($i = 0; $i < 3; $i++) {
                            $match[] = array($x, $y + $i);
                        }

                        $matches[] = $match;
                    }
    
                    // Check vertical (down)
                    if ($x <= $rows - 3 && $this->checkDirection($board, $x, $y, 1, 0)) {
                        $match = array();
                        
                        for ($i = 0; $i < 3; $i++) {
                            $match[] = array($x + $i, $y);
                        }

                        $matches[] = $match;
                    }
                }
            }
        }
    
        return $matches;
    }

    /**
     * put together an array of all x/y coords
     */
    private function getAllCoords() {
        $ret = array();

        foreach ($this->groups as $g) {
            $ret = array_merge($ret, $g->getPosition);
        }

        return $ret;
    }

    private function getFreeSpace($orientation) {
        $allCoords = $this->getAllCoords();
        $available = array();

        if ($orientation == Group::$orientationHorizontal) {
            // find a free horizontal position
            for ($x = 0; $x < $this->height; $x++) {
                if (!in_array([$x, 0], $allCoords) && !in_array([$x, 1], $allCoords)) {
                    $available[] = array([$x, 0], [$x, 1]);
                }
            }

            return $available[array_rand($available)];
        } else if ($orientation == Group::$orientationVertical) {
            // find a free vertical position
            for ($x = 0; $x < $this->height - 1; $x++) {
                if (!in_array([$x, 0], $allCoords) && !in_array([$x + 1, 0], $allCoords)) {
                    $available[] = array([$x, 0], [$x + 1, 0]);
                }
            }
        }
    }

    private function addGroup() {
        // pick a starting place and orientation
        $orientation = Group::$orientationHorizontal;

        // get a free spot to start the group
        $free = $this->getFreeSpace($orientation);

        $h1 = $free[0][0];
        $v1 = $free[0][1];
        $h2 = $free[1][0];
        $v2 = $free[1][1];

        echo "New group added at $h1, $v1   $h2, $v2\n";

        // create the new units
        $A = new Unit($h1, $v1);
        $B = new Unit($h2, $v2);

        $group = new Group($A, $B, $orientation);

        $this->groups[] = $group;

        // set the active group if it hasn't been already
        if (is_null($this->activeGroup)) {
            $this->activeGroup = count($this->groups) - 1;
        }
    }

    private function coordInGroups($h, $v, $gid = null) {
        for ($i = 0; $i < count($this->groups); $i++) {
            if ($i === $gid) {
                continue;
            }

            $g = $this->groups[$i];

            $pos = $g->getPosition();

            if (in_array([$h, $v], $pos)) {
                return true;
            }
        }
        
        return false;
    }

    private function coordInGrid($h, $v) {
        if (isset($this->grid[$h][$v]) && !is_null($this->grid[$h][$v])) {
            return true;
        }

        return false;
    }

    /**
     * make sure the two coordinates are free
     */
    public function checkMove($h1, $v1, $h2, $v2, $gid = null) {
        if (($h1 >= 0 && $h1 < $this->height) && ($v1 >= 0 && $v1 < $this->width) && ($h2 >= 0 && $h2 < $this->height) && ($v2 >= 0 && $v2 < $this->width)) {
            if ($this->coordInGroups($h1, $v1, $gid)) {
                echo "Found coord ($h1, $v1) in a group\n";

                return false;
            } else if ($this->coordInGrid($h1, $v1)) {
                echo "Found coord ($h1, $v1) in the grid\n";
                return false;
            } else if ($this->coordInGroups($h2, $v2, $gid)) {
                echo "Found coord ($h2, $v2) in a group\n";
                return false;
            } else if ($this->coordInGrid($h2, $v2)) {
                echo "Found coord ($h2, $v2) in the grid\n";
                return false;
            }

            return true;
        } else {
            echo "move is out of bounds\n";
            return false;
        }
    }

    /**
     * gets the XY coords, and character representation, for all groups
     */
    public function getAllPositionChars() {
        $ret = array();

        foreach ($this->groups as $g) {
            $pchars = $g->getPositionChars();

            foreach ($pchars as $char => $coords) {
                if (!isset($ret[$char])) {
                    $ret[$char] = array();
                }

                foreach ($coords as $coord) {
                    $ret[$char][] = $coord;
                }
            }
        }

        return $ret;
    }

    public function getScore() {
        return $this->score;
    }

    public function __toString() {
        //$ret = "aticveGroup: " . (!is_null($this->activeGroup) ? $this->activeGroup : "null") . "\n";

        $ret = "score: " . $this->getScore() . "\n";

        $lines = array();

        if (!is_null($this->activeGroup)) {
            $activePos = $this->groups[$this->activeGroup]->getPosition();
        } else {
            $activePos = array();
        }

        // get all the group data
        $groupChars = $this->getAllPositionChars();
       
        for ($x = 0; $x < $this->height; $x++) {
            $tmp = "";

            for ($y = 0; $y < $this->width; $y++) {
                $found = false;

                foreach ($groupChars as $char => $coords) {                   
                    foreach ($coords as $coord) {
                        if ([$x, $y] == $coord) {
                            if (in_array([$x, $y], $activePos)) {
                                $tmp .= Colors::fg_color('white', $char);
                            } else {
                                $tmp .= $char;
                            }

                            $found = true;
                        }
                    }
                }
                    
                if (!$found) {
                    if (!is_null($this->grid[$x][$y])) {
                        $tmp .= $this->grid[$x][$y];
                    } else {
                        $tmp .= " ";
                    }
                }
            }

            $lines[] = $tmp;
        }

        $output = Config::addBorder($lines);

        foreach ($output as $line) {
            $ret .= $line . "\n";
        }
        
        $ret .= "\n";

        return $ret;
    }
}