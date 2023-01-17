"use strict";

// external functions return a return code
// false - graceful completion, but no action was taken
// true - correct completion, actions were performed, for example, changes were made to the document
// string - erroneous termination, error message; OR the result of issuing a string

function isHdrTag(t) 
{
	if(t=='H1' 
	|| t=='H2'
	|| t=='H3'
	|| t=='H4'
	|| t=='H5'
	|| t=='H6'
	)
		return true;
	return false;
}

function isDivTag(t) 
{
	if(t=='P'
	|| t=='DIV'
	|| t=='PRE'
	|| t=='BLOCKQUOTE'
	|| t=='DETAILS'
	
	|| t=='FEATURE'
	|| t=='QUESTION'
	|| t=='IMPORTANT'
	|| t=='COMMENT'
	|| t=='ANN'
	|| t=='TERMINAL'
	|| t=='UNDERCONS'
	)	
		return true;
	return false;
}

function isSpanTag(t) 
{
	if(false
//	|| t=='SPAN' 
	|| t=='B' 
	|| t=='I' 
	|| t=='U' 
	|| t=='S'
	|| t=='SUB'
	|| t=='SUP'
	|| t=='STRONG'
	|| t=='EM'
	|| t=='INS'
	|| t=='DEL'
	|| t=='A'
	|| t=='CODE'
	|| t=='MARK'
	|| t=='Q'
	|| t=='SAMP'
	|| t=='VAR'
	|| t=='KBD'
	)
		return true;
	return false;
}

function isTableTag(t)
{
	if(t=='TABLE'
	|| t=='CAPTION'
	|| t=='THEAD'
	|| t=='TBODY'
	|| t=='TH'
	|| t=='TR'
	|| t=='TD'
	)
		return true;
	return false;
}

function isSpecTag(t)
{
	if(t=='IMG'
	|| t=='BR'
	|| t=='HR'
	|| t=='OL'
	|| t=='UL'
	|| t=='LI'
	)
		return true;
	if(isTableTag(t))
		return true;
	return false;
}

function isValidAttr(a)
{
	if(a=='href'
	|| a=='src'
	|| a=='colspan'
	|| a=='rowspan'
	|| a=='class'
	)
		return true;
	return false;
}

// debug - display the string in a special tag
function debugMsg(msg)
{
	var res = document.getElementById("res");
	res.innerHTML = msg;
}

function logNode(msg, node)
{
	if(node.nodeType == 3)
		console.log(msg + ' #text: ' + node.textContent);
	else
		console.log(msg + " " + node.tagName);
}

// debug - loads the html representation of the editable text into the viewport
function updateHtml() 
{
	var oDoc = document.getElementById("frameId");
    var oContent = document.createTextNode(oDoc.innerHTML);
    var oPre = document.getElementById("sourceText");
    oPre.innerHTML = "";
    oPre.appendChild(oContent);
}

// call to built-in editing tools
function formatDoc(sCmd, sValue) 
{
	document.execCommand(sCmd, false, sValue);
}

// insert an element after the given one (works for the old webkit, because the after property is not in it yet)
function insertAfter(newElement, targetElement) 
{
    var parent = targetElement.parentNode;
    if (parent.lastChild == targetElement) {
        parent.appendChild(newElement);
    } 
    else {
        parent.insertBefore(newElement, targetElement.nextSibling);
    }
}

// get the borders of the selection as a structure
// containing a start tag, an end tag, and a root tag
// cursor positions are not saved
function getSelectionBounds() 
{
	var selection = window.getSelection();
	if(selection == null)
		return null;
	var range = selection.getRangeAt(0);
	if(range == null)
		return null;
	var start = range.startContainer;
	var end   = range.endContainer;
	var root  = range.commonAncestorContainer;
	if(start.nodeName.toLowerCase() == 'body') 
		return null;
	if(start.nodeName == '#text') 
		start = start.parentNode;
	if(end.nodeName == '#text') 
		end = end.parentNode;
	if(start == end) 
		root = start;
	return {
		root: root,
		start: start,
		end: end
	};
	
	return null;
}

// getting the element by caret when there is no selection
// you can use getSelectionBounds() and check that start == end
function getCaretElement() 
{
	var selection = window.getSelection();
	if(!selection)
		return null;
	if(selection.toString() != "")
		return null;
	var container = selection.anchorNode;
	if(!container)
		return null;
	if( container.nodeType !== 3 ) { 
		// if the node type is not text
		return container;     
	}
	else {
		// return parent if text node - in this case, there is no tag...       
		return container.parentNode     
	} 
} 

// copy attributes from one tag to another
function copyAttrs(original, replacement)
{
	for (var i = 0, l = original.attributes.length; i < l; ++i) {
        var nodeName = original.attributes.item(i).nodeName;
        var nodeValue = original.attributes.item(i).nodeValue;
        replacement.setAttribute(nodeName, nodeValue);
    }
}

// replace the tag with another one while keeping the content
function replaceTag(original, tagName)
{
    var replacement = document.createElement(tagName);

    // take all the original attributes and transfer them to the replacement
	// by and large, this is not necessary, because we only have specific attributes
	// associated with special tags (href, src, colspan, rowspan)
    // copyAttrs(original, replacement);

    replacement.innerHTML = original.innerHTML;
    original.parentNode.replaceChild(replacement, original);
	return true;
}

function wrapSelectedText(tagName) 
{       
    var s = window.getSelection().getRangeAt(0);
	if(s.toString()=="")
		return false;
    var f = s.extractContents();
    var e = document.createElement(tagName);
    e.appendChild(f);
    s.insertNode(e);
	return true;
}

function rewrapSelection(tagName) 
{
	// ! no, it wraps the whole thing in a div, not the selection
	document.execCommand('formatBlock', false, 'DIV');
	wrapSelectedText(tagName);
}


// insert a character outside the current tag (out of the tag)
function insertOutside()
{
	var e = getCaretElement();
	if(!e)
		return "caret is null";
	if(e.tagName == 'LI')
	  e = e.parentNode;
	var newText = document.createTextNode("-");
	insertAfter(newText, e);
	
	var r = document.createRange();
	r.selectNodeContents(newText);
	
	var sel = window.getSelection(); 
    sel.removeAllRanges(); 
    sel.addRange(r); 
	return true;
}

function walkSelectionLevel(elem, end)
{
	while(elem != null) {
		if(elem == end)
			return false;
		if(walkSelectionLevel(elem.firstChild, end) == false)
			return false;
		elem = elem.nextSibling;
	}
	return true;
}

// check that there is no selection, but there is a caret
function isCaret()
{
	var selection = window.getSelection();
	if(!selection)
		return false;
	if(selection.toString() == "")
		return true;
	return false;
}

function isSelectionInSameLevel()
{
	// returns true if the selection is tagged and false otherwise
	var s = window.getSelection();
	if(s==null)
		return null;
	var r = s.getRangeAt(0);	// this is for ordering start and end
	if(r == null)
		return null;
	var e = r.startContainer;
	while(e != null) {
		if(e == r.endContainer)
			return true;
		if(walkSelectionLevel(e.firstChild, r.endContainer) == false)
			return false;
		e = e.nextSibling;
	}
	return false;
}


// wrap selected text in an inline tag if possible
function makeSpan(spanName) 
{
    //console.log("makeSpan: " + spanName);
	// if there is a selection, then check its correctness
	// if not, then perhaps this is the replacement of one span with another
	var se = getCaretElement();
	if (se) {
	    console.log("caret");
		if(se.tagName == spanName)
			return false;//unwrapTag(se);
		else if(isSpanTag(se.tagName))
			return replaceTag(se, spanName)
		return "not span tag";
	}
    else if (isSelectionInSameLevel()) {
        console.log("selection");    
		return wrapSelectedText(spanName);
	}
	else
		return "no caret or one-level selection"
}

// wrap selected text in a block tag if possible
function makeDiv(divName)
{
    //console.log("makeDiv: " + divName);
	// if there is a selection, then check its correctness
	// if not, then perhaps this is the replacement of one div with another
	var se = getCaretElement();
	if (se) {
	    console.log("caret");
		if(se.tagName == divName)
			return false;//unwrapTag(se);
		else if(isDivTag(se.tagName) || isHdrTag(se.tagName)) 
			return replaceTag(se, divName)
		return "not div/hdr tag";
	}
    else if (isSelectionInSameLevel()) {
        console.log("selection");    
		return wrapSelectedText(divName);
	}
	else
		return "no caret or one-level selection";
}

function unwrapTag(tag)
{
    var parent = tag.parentNode;
    while( tag.firstChild ) {
        parent.insertBefore(  tag.firstChild, tag );
    }
    parent.removeChild( tag );
	return true;
}

function unwrapCaretTag()
{
	var se = getCaretElement();
	if(se) {
		var t = se.tagName;
		if(t != 'BODY') {
			unwrapTag(se);
			return true;
		}
		return false;
	}
	return "not caret!";
}

function clearDocLevel(elem, ident)
{
	// clearing one level of the document
	var sum = 0;
//	var t = elem.tagName;
//	console.log(ident + t);
	
	// first go recursively through the elements
	var ch = elem.firstChild;
	while(ch) {
		var ch_next = ch.nextSibling;
		sum += clearDocLevel(ch, ident + " ");
		ch = ch_next;
	}

	// now we look at what kind of node; tag-only action
	if(elem.nodeType == Node.ELEMENT_NODE) {
		var n = elem.tagName;

		// if it is not text and is not included in the group of allowed tags, then we replace the tag with its content
		if (n != 'BODY' && !isDivTag(n) && !isHdrTag(n) && !isSpanTag(n) && !isSpecTag(n)) {
		    console.log(ident + "remove tag: " + n);
			unwrapTag(elem);
			sum++;
		}
		// if it is included - clear attributes
		else {
		    var arr = [];
		    for (var i = 0, l = elem.attributes.length; i < l; ++i) {
                var a = elem.attributes.item(i).nodeName;
                if (!isValidAttr(a))
                    arr.push(a);
            }
            for (var i = 0, l = arr.length; i < l; ++i) {
                elem.removeAttribute(arr[i]);    
				console.log(ident + "remove attr: " + arr[i]);
				sum++;
            }
		}
	}
	return sum;
}

// clear the entire document
function clearDoc()
{
    var sum = clearDocLevel(document.body, "");
	if(sum > 0)
		return true;
	return false;
}

// correction path to css
function setDocCSS(csspath)
{
	var links = document.head.getElementsByTagName("link");
	for ( var i = 0; i < links.length; i++ ) {
		var rel = links[i].getAttribute("rel");
		if(rel == 'stylesheet') {
			links[i].setAttribute("href", csspath);
			return true;
		}
	}
	// if not, insert
	var csslink = document.createElement("link");
	csslink.setAttribute("rel", "stylesheet");
	csslink.setAttribute("href", csspath);
	document.head.appendChild(csslink);
	return true;
}

// getting path to css
function getDocCSS()
{
	var links = document.getElementsByTagName("link");
	for ( var i = 0; i < links.length; i++ ) {
		var rel = links[i].getAttribute("rel");
		if(rel == 'stylesheet') {
			return links[i].getAttribute("href");
		}
	}
	return "";
}



// get the string of tags at the cursor
function getTags() 
{
    var se = getCaretElement();
    var ts = "";
    while (se && se.tagName != 'HTML') {
        if (se.tagName) {
            ts = se.tagName + " / " + ts;
        }
        se = se.parentNode;
    }
    return ts;
}

function getSubsequent(item)
{
	if(!item)
		return null;
	var next = item.firstChild;
	if(next)
		return next;
	next = item.nextSibling;
	if(next)
		return next;
	next = item.parentNode;
	while(next && !next.nextSibling)
		next = next.parentNode;
	if(next)
		return next.nextSibling;
	return null;
}

function walkSelectionSubtree(item, end, ident)
{
	if(item == null)
		return false;
	var curr_item = item;	// for functions that perform an action after the bypass
	
	while(item) {
		// display the next element
		logNode(ident + "item", item)
	
		var next_item = item.nextSibling;
		if(walkSelectionSubtree(item.firstChild, end, ident + " ") == true)
			return true;
		
		if(item == end)
			return true;
	
		item = next_item;
	}
	
	// 
	//unwrapTag(curr_item);
	
	return false;
}

function walkSelection(item, end, fn)
{
	// under construction, for border elements
	while(item) {
		logNode("loop: ", item)
		
		// new algorithm for selecting the next with a return to the parent elements
		var next_item = item.parentElement;
		while(next_item && next_item.nextSibling) 
			next_item = next_item.parentNode;
		if(next_item)
			next_item = next_item.nextSibling;
			
		if(walkSelectionSubtree(item, end, " "))
			return true;
				
		item = next_item;
	}
	return false;
}

function walkSelection2(item, end, fn)
{
	// working version, but the problem is that it's iterative
	// so it won't be possible to delete elements
	if(item == end)
		return true;
	var stop = false;
	while(item && !stop) {
		if(item == end)
			stop = true;
		logNode("item: ", item)
		var next_item = getSubsequent(item);
		if(fn)
			fn(item);	// no, we'll kill everything at once!
		item = next_item;
	}
	return stop;
}

function eraseSelection()
{
	// doesn't work yet
	var selection = window.getSelection();
	if(selection == null)
		return null;
	var range = selection.getRangeAt(0);
	if(range == null)
		return null;
	
	var start = range.startContainer;
	var end   = range.endContainer;
	
	var r = walkSelection2(start, end, unwrapTag);	// incorrect!
}

function tableNodesToText(node)
{
	// loop through child nodes
	var ch = node.firstChild;
	while(ch) {
		var nch = ch.nextSibling;
		tableNodesToText(ch);
		ch = nch;
	}
	// if this is a table node
	var tn = node.tagName;
	if(isTableTag(tn)) {
		unwrapTag(node);
	}
}

function tableToText()
{
	var se = getCaretElement();
	if(!se) 
		return "no caret!";
	// get the table tag
	while(se && se.tagName != 'TABLE')
		se = se.parentNode;
	if(!se)
		return "table not found!";
	// you need to delete not only table, but also all nested tags related to the table
	tableNodesToText(se);
	return true;
}

function insertTagAfter(node, tagName)
{
	// go beyond the tag
	// create a new node
	var newTag = document.createElement(tagName);
	insertAfter(newTag, node);
	newTag.textContent = tagName;
	//var newTxt = document.createTextNode(tagName);
	//insertAfter(newTxt, newTag);
	
	var r = document.createRange();
	r.selectNodeContents(newTag);
	
	var sel = window.getSelection(); 
	sel.removeAllRanges(); 
	sel.addRange(r);	
}

function makeDefItem()
{
	// workaround for new dt/dd item, handling 'enter'
	var se = getCaretElement();
	if(!se) 
		return "no caret!";
	// get the tag DD; if not, we are not in DL and do nothing
	if(se.tagName == 'DD') {
		insertTagAfter(se, 'DT');
		return true;		
	}
	else 
	if(se.tagName == 'DT') {	
		insertTagAfter(se, 'DD');
		return true;		
	}
	return false;
}

function getFocusText() 
{
    var text = "";
    if (window.getSelection().toString().length > 0) {
        console.log("1");
        console.log(window.getSelection().toString());
        text = window.getSelection().toString();
    }
    else if (document.selection && document.selection.type != "Control") {
        console.log("2");
        text = document.selection.createRange().text;
    }
    else {
        console.log("3");
        var sel = getSelectionBounds();
        console.log(sel.root);
        console.log(sel.start);
        console.log(sel.end);


        var node = document.activeElement;
        console.log(node);
        if (node && node.selectionStart >= 0) {
            var boundaries = {
                start: node.selectionStart,
                end: node.selectionStart
            };
            var range = document.createRange()
            range.selectNode(node)
            text = range.cloneContents().textContent
            if (text) {
                var i = 0;
                while (i < 1) {
                    var start = boundaries.start;
                    var end = boundaries.end;
                    var prevChar = text.charAt(start - 1);
                    var currentChar = text.charAt(end);

                    if (!prevChar.match(/\s/g) && prevChar.length > 0) {
                        boundaries.start--;
                    }

                    if (!currentChar.match(/\s/g) && currentChar.length > 0) {
                        boundaries.end++;
                    }

                    // if we haven't moved either boundary, we have our word
                   //  if (start === boundaries.start && end === boundaries.end) {
                   //     console.log('found!')
                    i = 1;
                }
            }
            text = text.slice(boundaries.start, boundaries.end)
        }
    }
    return text;
}


function FindWord(str, pos) 
{ 
	var L = str.length;
	var Le = L-pos;
	
	//1 is not on a space ? //
	if(str.substring(pos,pos+1).search(/\s/i)!=-1)
		return false;
	
	//2 define the beginning of the word
	for(var i=pos; i>-1; i--) {//alert(i)
		if(str.substring(i,pos).search(/\s/i)!=-1)
			break;
	}
	var st = 0;
	if(i) 
		st=i+1;
	for(var i=pos; i<L; i++){//alert(i)
		if(str.substring(i,pos).search(/\s/i)!=-1)
			break;
	}
	var end = L-1;
	if(i)
		end=i-1;
	return str.substring(st,end)
}

function getCaret()
{
	var ele = getCaretElement();
	console.log(ele);
	
	var sel = window.getSelection();
	var rng = sel.getRangeAt(0);

	console.log(rng);
	console.log(rng.startContainer.textContent)
	console.log(rng.startOffset)
	
	var wrd = FindWord(rng.startContainer.textContent, rng.startOffset);
	console.log(wrd);
}

function getCaretPosition () 
{
	const sel = document.getSelection()
	const r = sel.getRangeAt(0)
	let rect
	let r2
	let cleft = 0, ctop = 0
	let str = ""
	// supposed to be textNode in most cases
	// but div[contenteditable] when empty
	const node = r.startContainer
	const offset = r.startOffset
	if (offset > 0) {
		// new range, don't influence DOM state
		r2 = document.createRange()
		r2.setStart(node, (offset - 1))
		r2.setEnd(node, offset)
		// https://developer.mozilla.org/en-US/docs/Web/API/range.getBoundingClientRect
		// IE9, Safari?(but look good in Safari 8)
		rect = r2.getBoundingClientRect()
		cleft = rect.right;
		ctop = rect.top;
	} else if (offset < node.length) {
		r2 = document.createRange()
		// similar but select next on letter
		r2.setStart(node, offset)
		r2.setEnd(node, (offset + 1))
		rect = r2.getBoundingClientRect()
		cleft = rect.left
		ctop = rect.top
	} else { // textNode has length
		// https://developer.mozilla.org/en-US/docs/Web/API/Element.getBoundingClientRect
		rect = node.getBoundingClientRect()
		const styles = getComputedStyle(node)
		const lineHeight = parseInt(styles.lineHeight)
		const fontSize = parseInt(styles.fontSize)
		// roughly half the whitespace... but not exactly
		const delta = (lineHeight - fontSize) / 2
		cleft =  rect.left
		ctop = rect.top + delta
	}
	str += cleft;
	str += ';';
	str += ctop;
	return str;
}
	


