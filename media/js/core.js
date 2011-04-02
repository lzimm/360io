var io360 = {}
io360.cookies = {}

io360.core = {
    init : function(postInitHook) {
        io360.core.commRoot = io360.util.base62(new Date().getTime().toString().substring(5)) + ".";
        io360.core.commRoot = "";
        
        document.domain = "360.io";
        
        $("#_links").hover(function() {
            $("#_links").fadeTo(125, 1);
        }, function() {
            $("#_links").fadeTo(125, 0);
        });
        
        $("#_links").fadeTo(125, 0);
        
        $("div._floater").each(function() {
            if (!$(this).hasClass("_free")) {
                $(this).appendTo($("#_floatplate").clone().attr("id", "_float" + $(this).attr("id")).appendTo("body").find("div._internal"));
                $("#_float" + $(this).attr("id")).css({
                    //marginTop: (-1 * $("#_float" + $(this).attr("id")).height()/2) + "px"
                }).find("h1 span").html($(this).attr("id").substr(1));
            }
            
            $(this).show();
        });
        
        io360.draw.init();
        io360.style.init();
            
        io360.xdswf.init(function() {
            io360.crossDomain.cookie(
                    "http://service.360.io/crossdomain/bridge.html",
                    function(data) {
                        io360.logging.log("initCookies", data);
                    
                        var ca = data.split(';');
                    	for (var i = 0; i < ca.length; i++) {
                    		var c = ca[i];
                    		while (c.charAt(0) == ' ') c = c.substring(1, c.length);
                    		io360.cookies[c.substring(0, c.indexOf('='))] = c.substring(c.indexOf('=') + 1, c.length);
                    	}
                	
                    	if (io360.cookies.auth) {
                    	    io360.auth.uid = io360.cookies.user_id;
                    	    io360.auth.uname = io360.cookies.user_name;
                    	    io360.auth.utwitter = io360.cookies.twitter;
                	    
                    	    if (io360.auth.utwitter == "None") {
                    	        io360.auth.utwitter = false;
                    	    }
                	                    	    
                    	    var oldInitHook = postInitHook;
                    	    postInitHook = function() {
                                io360.commands.authCommSession(oldInitHook);
                            };
                    	}
                	
                    	io360.auth.refreshView();
                	
                    	if (!io360.engine.sessionID) {
                    	    io360.commands.initCommSession(io360.commands.connectCommSession, [postInitHook]);
                	    } else {
                	        io360.commands.connectCommSession(postInitHook);
                	    }
            	    
                    	io360.core.registerInputs();
                    }
                );
        });
    },
    
    registerInputs : function() {
        $("#_publisher input, #_auth input, #_publisher textarea").each(function() {
            $(this).attr("default", $(this).val());
        });
        
        $("#_publisher input, #_auth input, #_publisher textarea").focus(function() {
            if ($(this).val() == $(this).attr("default")) {
                $(this).val("");
            }
        });
        
        $("#_publisher input, #_auth input, #_publisher textarea").blur(function() {
            if ($(this).val() == "") {
                $(this).val($(this).attr("default"));
            }            
        });
    }
}

io360.xdswf = {
    transport : null,
    initCallback : null,
    swfdiv : null,
    
    detect : function() {
        var UNDEF = "undefined",
            OBJECT = "object",
            SHOCKWAVE_FLASH = "Shockwave Flash",
            SHOCKWAVE_FLASH_AX = "ShockwaveFlash.ShockwaveFlash",
            FLASH_MIME_TYPE = "application/x-shockwave-flash",
            win = window,
            doc = document,
            nav = navigator;
        
        var w3cdom = typeof doc.getElementById != UNDEF && typeof doc.getElementsByTagName != UNDEF && typeof doc.createElement != UNDEF,
            u = nav.userAgent.toLowerCase(),
            p = nav.platform.toLowerCase(),
            windows = p ? /win/.test(p) : /win/.test(u),
            mac = p ? /mac/.test(p) : /mac/.test(u),
            webkit = /webkit/.test(u) ? parseFloat(u.replace(/^.*webkit\/(\d+(\.\d+)?).*$/, "$1")) : false, // returns either the webkit version or false if not webkit
            ie = !+"\v1", // feature detection based on Andrea Giammarchi's solution: http://webreflection.blogspot.com/2009/01/32-bytes-to-know-if-your-browser-is-ie.html
            playerVersion = [0,0,0],
            d = null;
            
        if (typeof nav.plugins != UNDEF && typeof nav.plugins[SHOCKWAVE_FLASH] == OBJECT) {
            d = nav.plugins[SHOCKWAVE_FLASH].description;
            if (d && !(typeof nav.mimeTypes != UNDEF && nav.mimeTypes[FLASH_MIME_TYPE] && !nav.mimeTypes[FLASH_MIME_TYPE].enabledPlugin)) { // navigator.mimeTypes["application/x-shockwave-flash"].enabledPlugin indicates whether plug-ins are enabled or disabled in Safari 3+
                plugin = true;
                ie = false; // cascaded feature detection for Internet Explorer
                d = d.replace(/^.*\s+(\S+\s+\S+$)/, "$1");
                playerVersion[0] = parseInt(d.replace(/^(.*)\..*$/, "$1"), 10);
                playerVersion[1] = parseInt(d.replace(/^.*\.(.*)\s.*$/, "$1"), 10);
                playerVersion[2] = /[a-zA-Z]/.test(d) ? parseInt(d.replace(/^.*[a-zA-Z]+(.*)$/, "$1"), 10) : 0;
            }
        } else if (typeof win.ActiveXObject != UNDEF) {
            try {
                var a = new ActiveXObject(SHOCKWAVE_FLASH_AX);
                if (a) { // a will return null when ActiveX is disabled
                    d = a.GetVariable("$version");
                    if (d) {
                        ie = true; // cascaded feature detection for Internet Explorer
                        d = d.split(" ")[1].split(",");
                        playerVersion = [parseInt(d[0], 10), parseInt(d[1], 10), parseInt(d[2], 10)];
                    }
                }
            } catch(e) {}
        }
        
        return playerVersion[0] >= 9;
    },
    
    init : function(callback) {
        if (!io360.xdswf.detect()) {
            io360.crossDomain.request = io360.crossDomain.ifrRequest;
            callback();
            return;
        }
        
        io360.xdswf.initCallback = callback;
        
        var swfurl = 'http://media.360.io/xd.swf';
        io360.xdswf.swfdiv = $('<div>').html(" \
                <object classid='clsid:D27CDB6E-AE6D-11cf-96B8-444553540000' data='"+swfurl+"' name='xdswf' id='xdswf' type='application/x-shockwave-flash' height='100%' width='100%'> \
                <param value='always' name='allowscriptaccess' /><param value='"+swfurl+"' name='movie' /> \
                <param name='wmode' value='transparent' /> \
                <embed id='xdswf' name='xdswf' type='application/x-shockwave-flash' src='"+swfurl+"' width='100%' height='100%' wmode='transparent' allowscriptaccess='always' /> \
                </object>").css({position: 'absolute', top: 0, left: 0, width: 1, height: 1});
        io360.xdswf.swfdiv.appendTo("body");
    },
    
    ready : function() {
        if(navigator.appName.indexOf("Microsoft") != -1) {
            io360.xdswf.transport = window['xdswf'];
        } else {
            io360.xdswf.transport = document.getElementById('xdswf');
        }

        io360.xdswf.initCallback();
    },
    
    request : function(path, args, callback) {
        io360.xdswf.transport.request(path, args, callback);
    },
    
    initComm : function(channel, callback) {
        io360.xdswf.transport.initComm(channel, callback);
    },
    
    uneval : function(o) {
        switch (typeof o) {
            case "undefined" : return "(void 0)";
            case "boolean"   : return String(o);
            case "number"    : return String(o);
            case "string"    : return '"' + o.replace(/"/g, '\\"') + '"';
            case "function"  : return "(" + o.toString() + ")";
            case "object"    :
                if (o == null) return "null";
                var type = Object.prototype.toString.call(o).match(/\[object (.+)\]/);
                if (!type) throw TypeError("unknown type:"+o);
                switch (type[1]) {
                    case "Array":
                        var ret = [];
                        for (var i = 0; i < o.length; i++) ret.push(arguments.callee(o[i]));
                        return "[" + ret.join(", ") + "]";
                    case "Object":
                        var ret = [];
                        for (var i in o) {
                            if (!o.hasOwnProperty(i)) continue;
                            ret.push(arguments.callee(i) + ":" + arguments.callee(o[i]));
                        }
                        return "{" + ret.join(", ") + "}";
                    case "Number":
                        return "(new Number(" + o + "))";
                    case "String":
                        return "(new String(" + arguments.callee(o) + "))";
                    case "Date":
                        return "(new Date(" + o.getTime() + "))";
                    default:
                        if (o.toSource) return o.toSource();
                        throw TypeError("unknown type:"+o);
                }
        }
    }
}

io360.parse = {
    data : function(jsonData) {
        var data = jsonData;
        while (typeof(data) == typeof("string")) {
            data = eval("(" + data + ");");
        }
        return data;
    }
}

io360.logging = {
    log : function(heading, body) {
        $("#stage").prepend("<div class='log'><h3>" + heading + "</h3><p>" + body + "</p></div>");
        $("#stage div.log:first").show("fast");
        
        try {
            if (console && console.log) {
                console.log("(" + heading + "): " + body);
            }
        } catch (e) {}
    }
}  

io360.util = {
    formsEnabled : true,
    
    enableForms : function() {
        $("form").each(function() {
            $(this).removeClass("_disabled");
            $("input", this).attr("disabled", false);
            $("textarea", this).attr("disabled", false);
        });
        
        io360.util.formsEnabled = true;
    },
    
    disableForms : function() {
        io360.util.formsEnabled = false;
        
        $("form").each(function() {
            $(this).addClass("_disabled");
            $("input", this).attr("disabled", "disabled");
            $("textarea", this).attr("disabled", "disabled");
        });        
    },
    
    setCookie : function(name, value) {
        if (!value) value = '';
        document.cookie = name + '=' + value + '; path=/;';
    },
    
    base62 : function(input) {
         charmap = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-";
         var ret = "";
         var acc = 0;
         var div = 1;
         
         for(var i = 0; i < input.length; i++) {
             acc = acc*248 + input.charCodeAt(i);
             div = div*4;
             ret = ret + charmap.charAt(parseInt(acc/div));
             acc = acc % div;
             if (div == 62) ret = ret + charmap.charAt(parseInt(acc)), acc = 0, div = 1;
         } 

         return ret;
    }
}

io360.crossDomain = {
    context : {},
    curID : 0,
    
    callbacks : {},
    commChannel : null,
    commCallback : null,
    
    request : function(domain, path, args, callback, passthrough) {
        if (path.indexOf("comm.360.io/LTN/") < 0) {
            var id = io360.crossDomain.curID++;
        
            var wrappedCallback = function(data) {
                callback(data);
                if (!passthrough) {
                    io360.draw.hideLoader();
                }
                delete io360.crossDomain[id];
            }

            if (!passthrough) {
                io360.draw.showLoader();
            }

            io360.crossDomain.callbacks[id] = wrappedCallback;
            io360.xdswf.request(path, args, "io360.crossDomain.callbacks[" + id + "]");
        } else {
            if (path != io360.crossDomain.commChannel) {
                io360.crossDomain.commChannel = path;
                io360.crossDomain.commCallback = callback;
                io360.xdswf.initComm(path, "io360.crossDomain.commCallback");
            }
        }
    },

    ifrRequest : function(domain, path, args, callback, passthrough) {
        var iframe = document.createElement("iframe");
        var id = io360.crossDomain.curID++;
        
        iframe.style.position = "absolute";
        iframe.style.width = iframe.style.height = "1px";
        iframe.style.top = iframe.style.left = "-10000px";
        iframe.src = domain + "#" + id;
        
        var wrappedCallback = function(data) {
            callback(data);
            if (path.indexOf("comm.360.io/LTN/") < 0 && !passthrough) {
                io360.draw.hideLoader();
            }
        }
        
        io360.crossDomain.context[id] = {
            path : path,
            callback : wrappedCallback,
            args : args
        };
        
        if (path.indexOf("comm.360.io/LTN/") < 0 && !passthrough) {
            io360.draw.showLoader();
        }
        
        (document.body || document.documentElement).appendChild(iframe);
    },
    
    cookie : function(domain, command) {
        var iframe = document.createElement("iframe");
        var id = io360.crossDomain.curID++;
        
        iframe.style.position = "absolute";
        iframe.style.width = iframe.style.height = "1px";
        iframe.style.top = iframe.style.left = "-10000px";
        iframe.src = domain + "#" + id;
        
        io360.crossDomain.context[id] = {
            cookieHandler : command
        };
        
        (document.body || document.documentElement).appendChild(iframe);        
    }
}

io360.listeners = {
    echo : function(data) {
        io360.logging.log("defaultListener", data);
    },
    
    reply : function(data) {
        var users = {};
        for (var i = 0; i < data.users.length; i++) {
            var user = data.users[i];
            if (user && user != "null") {
                users[user.id] = user;
            }
        }
        
        for (var i = 0; i < data.tail.length; i++) {
            var reply = data.tail[i];
            var user = users[reply.uid];
            user = user ? user : io360.parse.data(reply.user);

            io360.draw.notification("Reply", user.name + " posted a reply to one of your comments", function() {
                if (io360.stream && io360.stream.lid == reply.lid) {
                    io360.stream.comment.show(reply.id);
                } else {
                    io360.logging.log("reply", "http://360.io/" + reply.lid + "#" + reply.id);
                    window.open("http://360.io/" + reply.lid + "#" + reply.id, reply.lid + "#" + reply.id);
                }
            });
        }
    }
}

io360.commands = {
    initCallback : null,
    initArgs : null,
    
    initCommSession : function(callback, args) {
        io360.commands.initCallback = callback;
        io360.commands.initArgs = args;
        
        io360.crossDomain.request(
                "http://" + io360.core.commRoot + "comm.360.io/BRG/",
                "http://" + io360.core.commRoot + "comm.360.io/NEW", 
                {}, 
                function(data) {
                    io360.logging.log("initCommSession", "sessionID: " + data);
                    io360.engine.sessionID = data;
                    callback.apply(this, args ? args : []);
                });
    },
    
    connectCommSession : function(callback, args) {
        io360.crossDomain.request(
                "http://" + io360.core.commRoot + "comm.360.io/BRG/",
                "http://" + io360.core.commRoot + "comm.360.io/LTN/" + io360.engine.sessionID, 
                {},
                function(data) {
                    if (data == "error") {
                        io360.commands.initCommSession(io360.commands.initCallback, io360.commands.initArgs);
                        return;
                    }
                    
                    io360.commands.connectCommSession();
                    io360.logging.log("connectCommSession", data);
                    
                    if (data == "reconnect") return;
                    
                    data = io360.parse.data(data);
                    if (data instanceof Array) {
                        for (var i = 0; i < data.length; i++) {
                            for (var lbl in data[i]) {
                                if (io360.listeners[lbl]) {
                                    io360.listeners[lbl](data[i][lbl]);
                                } else {
                                    io360.listeners.echo(data[i][lbl]);
                                }
                            }
                        }                    
                    } else {
                        for (var lbl in data) {
                            if (io360.listeners[lbl]) {
                                io360.listeners[lbl](data[lbl]);
                            } else {
                                io360.listeners.echo(data[lbl]);
                            }
                        }
                    }
                });
        
        if (callback) { callback.apply(this, args ? args : []); }
    },
    
    registerCommSession : function(channel) {
        if (!io360.engine.sessionID) {
            setTimeout(function() { io360.commands.registerCommSession(channel); }, 1000);
        } else {
            io360.crossDomain.request(
                    "http://service.360.io/crossdomain/bridge.html",
                    "http://service.360.io/comm/register/" + io360.engine.sessionID, 
                    {
                        channel : channel
                    },
                    function(data) {
                        io360.logging.log("registerCommSession", data);
                    });   
        }
    },
    
    authCommSession : function(callback) {
        if (!io360.engine.sessionID) {
            setTimeout(function() { io360.commands.authCommSession(callback); }, 1000);
        } else {
            io360.crossDomain.request(
                    "http://service.360.io/crossdomain/bridge.html",
                    "http://service.360.io/comm/auth/" + io360.engine.sessionID, 
                    {
                    },
                    function(data) {
                        io360.logging.log("registerCommSession", data);
                        
                        if (callback) {
                            callback();
                        }
                    });   
        }        
    }
}

io360.engine = {
    sessionID : null
}

io360.draw = {
    loaderCount : 0,
    
    init : function() {
        this.initLoader();
        this.initNotifications();
    },
    
    initLoader : function() {        
        var loaderContainer = document.createElement('div');
        loaderContainer.id = "io360_loader";
        loaderContainer.innerHTML = "<b></b><div id='io360_loader_bg'></div><div id='io360_loader_fg'><span>360io</span></div><b></b>";
           
        document.getElementsByTagName("body")[0].appendChild(loaderContainer); 
        
        var spriteWidth = 10;
        var spriteMargin = 5;
        var spriteCount = (125 - spriteMargin - document.getElementById("io360_loader_fg").getElementsByTagName("span")[0].offsetWidth)/(spriteWidth + spriteMargin);
        var spriteBody = "";
        for (var i = 0; i < spriteCount; i++) {
            spriteBody += "<b></b>";
        }

        document.getElementById("io360_loader_fg").innerHTML += spriteBody;
        document.getElementById("io360_loader").setAttribute("step", 0);
    },
    
    initNotifications : function() {
        var queue = document.createElement('div');
        queue.id = "io360_notice_queue";
        queue.style.position = "absolute";
        queue.style.top = "0px";
        queue.style.right = "0px";
        queue.style.width = "200px";
        queue.style.paddingTop = "10px";
        queue.style.display = "block";
        queue.style.zIndex = "9999999999";

        document.getElementsByTagName("body")[0].appendChild(queue);
    
        var alerts = document.createElement('div');
        alerts.id = "io360_alerts";
        alerts.style.position = "absolute";
        alerts.style.top = "0px";
        alerts.style.left = "50%";
        alerts.style.width = "0px";
        alerts.style.paddingTop = "10px";
        alerts.style.display = "block";
        alerts.style.textAlign = "center";
        alerts.style.zIndex = "9999999999";

        document.getElementsByTagName("body")[0].appendChild(alerts);
    },
      
    el : function(id) { 
        return document.getElementById(id);
    },
    
    showLoader : function() {
        io360.draw.loaderCount++;
        if (io360.draw.loaderCount > 1) return;
        
        document.getElementById("io360_loader").style.display = "block";
        
        animate = function() {
            var sprites = document.getElementById("io360_loader_fg").getElementsByTagName("b");
            var stepsPerSprite = 5;

            var step = (parseInt(document.getElementById("io360_loader").getAttribute("step")) + 1) % (sprites.length * stepsPerSprite);
            document.getElementById("io360_loader").setAttribute("step", step);

            var sprite = parseInt(step/stepsPerSprite); 
            var stepProgress = step/stepsPerSprite;
            var falloffPerSprite = (150 / stepsPerSprite);
            var topAlpha = 100;

            function generateAlpha(i) {
                var spriteOffset = Math.abs(sprite - i);
                var delta = falloffPerSprite * spriteOffset;
                return topAlpha - delta;
            }

            function updateAlpha(sprite, alpha) {
                while (sprite < 0) {
                    sprite += sprites.length;
                }

                sprite = sprite % sprites.length;

                sprites[sprite].style.filter = 'progid:DXImageTransform.Microsoft.Alpha(Opacity=' + alpha + ')';
                sprites[sprite].style.opacity = alpha/100;
            }

            var firstSprite = sprite - parseInt(sprites.length / 2);
            var lastSprite = sprite + parseInt(sprites.length / 2);
            for (i = firstSprite; i < lastSprite; i++) {
                updateAlpha(i, generateAlpha(i));
            }

            document.getElementById("io360_loader").setAttribute("timeout", setTimeout(animate, 25));
        }
        
        animate();        
    },
    
    hideLoader : function() {
        io360.draw.loaderCount--;
        if (io360.draw.loaderCount > 0) return;
        
        document.getElementById("io360_loader").style.display = "none";
        
        var timeout = document.getElementById("io360_loader").getAttribute("timeout");
        if (timeout) {
            clearTimeout(timeout);
        }
    },
    
    notification : function(title, message, cb) {
        var noticeBody = document.createElement('div');
        noticeBody.className = "io360_notice_body";
        noticeBody.innerHTML = "<h2>"+title+"</h2><p>"+message+"</p>";
        
        var noticeBG = document.createElement('div');
        noticeBG.className = "io360_notice_bg";
        
        var notice = document.createElement('div');
        notice.className = "io360_notice";
        
        notice.appendChild(noticeBG);
        notice.appendChild(noticeBody);
        
        var noticeWrapper = document.createElement('div');
        noticeWrapper.className = "io360_notice_wrapper";
        noticeWrapper.appendChild(document.createElement('b'));
        noticeWrapper.appendChild(notice);
        noticeWrapper.appendChild(document.createElement('b'));
 
        document.getElementById("io360_notice_queue").appendChild(noticeWrapper);
        document.getElementById("io360_notice_queue").style.top = 
                    (window.pageYOffset ||
                        document.body.scrollTop ||
                        document.documentElement.scrollTop) + "px";
        
        noticeBG.style.height = noticeBody.offsetHeight + "px";
        notice.style.height = noticeBody.offsetHeight + "px";
        
        noticeWrapper.setAttribute("alpha", 100);
        noticeWrapper.setAttribute("height", noticeBody.offsetHeight + 2);
        
        var animation = function() {
            var alpha = parseInt(noticeWrapper.getAttribute("alpha")) + (0 - parseInt(noticeWrapper.getAttribute("alpha"))) / 2;
            var height = noticeWrapper.getAttribute("height");
            
            noticeWrapper.setAttribute("alpha", alpha);
            
            noticeWrapper.style.filter = 'progid:DXImageTransform.Microsoft.Alpha(Opacity=' + alpha + ')';
            noticeWrapper.style.opacity = alpha/100;
            
            noticeWrapper.style.height = (alpha/100 * height) + "px";
            noticeWrapper.style.marginBottom = (alpha/100 * 10) + "px";
            
            if (alpha > 1) {
                noticeWrapper.setAttribute("timeout", setTimeout(animation, 50));
            } else {
                document.getElementById("io360_notice_queue").removeChild(noticeWrapper);
            }
        };
        
        noticeWrapper.onmouseout = function() { 
                noticeWrapper.setAttribute("timeout", setTimeout(animation, 3000)); 
                noticeWrapper.className = "io360_notice_wrapper";
            };
            
        noticeWrapper.onmouseover = function() { 
                noticeWrapper.setAttribute("alpha", 100); 
                clearTimeout(noticeWrapper.getAttribute("timeout")); 
                
                noticeWrapper.style.filter = 'progid:DXImageTransform.Microsoft.Alpha(Opacity=100)';
                noticeWrapper.style.opacity = 1;
                
                noticeWrapper.style.height = noticeWrapper.getAttribute("height") + "px";
                noticeWrapper.style.marginBottom = "10px";
                
                noticeWrapper.className = "io360_notice_wrapper io360_notice_wrapper_hover";
            };
            
        if (cb) {
            noticeWrapper.style.cursor = "pointer";
            noticeWrapper.onclick = function() { cb(); }
        }
        
        noticeWrapper.setAttribute("timeout", setTimeout(animation, 3000));
    },
    
    alert : function(title, message, cb) {
        var alertBody = document.createElement('div');
        alertBody.className = "io360_alert_body";
        alertBody.innerHTML = "<h2>"+title+"</h2><p>"+message+"</p>";
        
        var alertBG = document.createElement('div');
        alertBG.className = "io360_alert_bg";
        
        var alert = document.createElement('div');
        alert.className = "io360_alert";
        
        alert.appendChild(alertBG);
        alert.appendChild(alertBody);
        
        var alertWrapper = document.createElement('div');
        alertWrapper.className = "io360_alert_wrapper";
        alertWrapper.appendChild(document.createElement('b'));
        alertWrapper.appendChild(alert);
        alertWrapper.appendChild(document.createElement('b'));
 
        document.getElementById("io360_alerts").appendChild(alertWrapper);
        document.getElementById("io360_alerts").style.top = 
                    (window.pageYOffset ||
                        document.body.scrollTop ||
                        document.documentElement.scrollTop) + "px";
        
        alertBG.style.height = alertBody.offsetHeight + "px";
        alert.style.height = alertBody.offsetHeight + "px";
        
        alertWrapper.setAttribute("alpha", 100);
        alertWrapper.setAttribute("height", alertBody.offsetHeight + 2);
        
        var animation = function() {
            var alpha = parseInt(alertWrapper.getAttribute("alpha")) + (0 - parseInt(alertWrapper.getAttribute("alpha"))) / 2;
            var height = alertWrapper.getAttribute("height");
            
            alertWrapper.setAttribute("alpha", alpha);
            
            alertWrapper.style.filter = 'progid:DXImageTransform.Microsoft.Alpha(Opacity=' + alpha + ')';
            alertWrapper.style.opacity = alpha/100;
            
            alertWrapper.style.height = (alpha/100 * height) + "px";
            alertWrapper.style.marginBottom = (alpha/100 * 10) + "px";
            
            if (alpha > 1) {
                alertWrapper.setAttribute("timeout", setTimeout(animation, 50));
            } else {
                document.getElementById("io360_alerts").removeChild(alertWrapper);
            }
        };
        
        alertWrapper.onmouseout = function() { 
                alertWrapper.setAttribute("timeout", setTimeout(animation, 3000)); 
                alertWrapper.className = "io360_alert_wrapper";
            };
            
        alertWrapper.onmouseover = function() { 
                alertWrapper.setAttribute("alpha", 100); 
                clearTimeout(alertWrapper.getAttribute("timeout")); 
                
                alertWrapper.style.filter = 'progid:DXImageTransform.Microsoft.Alpha(Opacity=100)';
                alertWrapper.style.opacity = 1;
                
                alertWrapper.style.height = alertWrapper.getAttribute("height") + "px";
                alertWrapper.style.marginBottom = "10px";
                
                alertWrapper.className = "io360_alert_wrapper io360_alert_wrapper_hover";
            };
        
        if (cb) {
            alertWrapper.style.cursor = "pointer";
            alertWrapper.onclick = function() { cb(); }
        }
        
        alertWrapper.setAttribute("timeout", setTimeout(animation, 3000));
    }
}

io360.floater = {
    showing : {},
    
    show : function(panel) {
        $("#_float_" + panel).fadeIn(125);
        io360.floater.showing[panel] = true;
    },
    
    hide : function(panel) {
        if ($(panel).length) {
            $(panel).parent().fadeOut(125);
            io360.floater.showing[$(panel).parent().attr("id").substr(7)] = false;
        } else {
            $("#_float_" + panel).fadeOut(125);
            io360.floater.showing[panel] = false;
        }
    }
}

io360.auth = {
    mode : "login",
    
    uid : false,
    uname : false,
    utwitter : false,
    
    toggle : function() {
        if (io360.floater.showing.auth) {
            io360.auth.close();
        } else {
            io360.auth.show();
        }
    },
    
    show : function() {
        io360.floater.show("auth");
    },
    
    close : function() {
        io360.floater.hide("auth");
    },
    
    refreshView : function(static) {
        if (io360.auth.uname) {
            $("#_auth #name").hide();
            $("#_auth #email").hide();
            $("#_auth #password").hide();
            
            $("#_auth p").html("Are you sure you want to logout?");
            $("#_authbtn span i").html("Logout");
            
            $("#_auth #swap").html(io360.auth.uname);
            
            $('#_auth').addClass('_authed'); 
            $("#_auth ._inputs span").html(io360.auth.uname);
            
            $('#_publisher ._inputs').addClass('_authed');
            $("#_publisher ._inputs span").html(io360.auth.uname);
            
            if (io360.auth.utwitter) {
                $("._tetherTwitter").hide();
            } else {
                $("._tetherTwitter").show();
            }
            
            io360.auth.close();
            io360.style.refresh();
            
            $("a._btn_auth").addClass("_btn_auth_logout").removeClass("_btn_auth_login");
            $("a._btn_dash").fadeIn(125);
        } else {
            $("#_auth #name").show();
            $("#_auth #email").show();
            $("#_auth #password").show();
            
            if (io360.auth.mode == "register") {
                $("#_auth #swap").html("Login");
                
                $("#_auth p").html("Enter your screen name, email and password to register");
                $("#_authbtn span i").html("Register");
                
                $("#_auth #name").show();
            } else {
                $("#_auth #swap").html("Register");
                
                $("#_auth p").html("Enter your email and password to login");
                $("#_authbtn span i").html("Login");
                
                $("#_auth #name").hide();
            }
            
            $('#_auth').removeClass('_authed'); 
            $("#_auth ._inputs span").html("");
            
            $('#_publisher ._inputs').removeClass('_authed');
            $("#_publisher ._inputs span").html("");
            
            $("._tetherTwitter").hide();
            
            io360.dash.close();
            if (!static) io360.auth.close();
            io360.style.current = "";
            io360.style.reset();
            
            $("a._btn_auth").addClass("_btn_auth_login").removeClass("_btn_auth_logout");
            $("a._btn_dash").fadeOut(125);
        }
    },
    
    auth : function() {
        if (io360.auth.uname) {
            io360.auth.logout();
        } else if (io360.auth.mode == "login") {
            io360.auth.login();
        } else {
            io360.auth.register();
        }
    },
    
    swapMode : function() {
        if (io360.auth.uname) {
            io360.auth.logout();
        } else if (io360.auth.mode == "login") {
            io360.auth.mode = "register";
        } else {
            io360.auth.mode = "login";
        }
        
        io360.auth.refreshView(true);
    },
    
    login : function() {
        var email = $("#_auth #email").val();
        var password = $("#_auth #password").val();   
        
        if (!io360.util.formsEnabled) return;
        io360.util.disableForms();
        
        io360.crossDomain.request(
                "http://service.360.io/crossdomain/bridge.html",
                "http://service.360.io/auth/login", 
                {
                    email : email,
                    password : password
                },
                function(data) {
                    io360.logging.log("io360.core.login", data);
                    io360.commands.authCommSession();
                    
                    data = io360.parse.data(data);
                    if (data) {
                        io360.auth.uid = data['id']
                        io360.auth.uname = data['name'];
                        io360.auth.utwitter = data['twitter'];
                        
                        io360.auth.refreshView();
                        
                        io360.draw.alert("Login", "Logged in as " + data['name']);          
                    } else {
                        io360.auth.uid = false;
                        io360.auth.uname = false;
                        io360.auth.utwitter = false;
                        
                        io360.draw.alert("Login Failure", "Failed to login");
                    }
                    
                    io360.util.enableForms();
                });        
    },
    
    register : function() {
        var name = $("#_auth #name").val();
        var email = $("#_auth #email").val();
        var password = $("#_auth #password").val();   
        
        io360.crossDomain.request(
                "http://service.360.io/crossdomain/bridge.html",
                "http://service.360.io/auth/register", 
                {
                    name : name,
                    email : email,
                    password : password
                },
                function(data) {
                    io360.logging.log("io360.core.register", data);
                    io360.commands.authCommSession();
                    
                    data = io360.parse.data(data);
                    if (data) {
                        io360.auth.uid = data['id'];
                        io360.auth.uname = name;
                        io360.auth.utwitter = false;
                        
                        io360.auth.refreshView();
                        io360.auth.mode = "login";
                        
                        io360.draw.alert("Register", "Registered as " + name);                       
                    } else {
                        io360.auth.uid = false;
                        io360.auth.uname = false;
                        io360.auth.utwitter = false;
                        
                        io360.draw.alert("Registration Failure", "Problem with your email address"); 
                    }
                });        
    },
    
    logout : function() {
        io360.crossDomain.request(
                "http://service.360.io/crossdomain/bridge.html",
                "http://service.360.io/auth/logout", 
                {
                },
                function(data) {
                    io360.logging.log("io360.core.logout", data);
                    io360.commands.authCommSession();

                    io360.auth.uid = false;
                    io360.auth.uname = false;
                    io360.auth.utwitter = false;
                    
                    io360.auth.refreshView(true);
                    
                    io360.draw.alert("Logout", "Logged Out"); 
                });        
    },
    
    settings : function() {
        io360.dash.show();
    }
}

io360.twitter = {
    window : null,

    show : function(link) {
        io360.dash.close();
        
        $("#_tweet p").html("Post to twitter");
        $("#_tweet textarea.comment").val(link);
        io360.twitter.update();
        io360.auth.refreshView();
        
        io360.floater.show("tweet");
    },
    
    close : function() {
        io360.floater.hide("tweet");
    },
    
    update : function() {
        var comment = io360.auth.utwitter ? " <strong>(Posting as: " + io360.auth.utwitter + ")</strong>" : "";
        var len = $("#_tweet textarea.comment").val().length;
        $("#_tweet ._count").html(len + " / " + 140 + comment);
    },

    tether : function() {
        if (io360.auth.uname) {
            io360.twitter.window = window.open(
                'http://service.360.io/auth/twitter/tether', 
                'twitter', 
                'location=0,status=0,scrollbars=0,width=800,height=350');
        }
    },
    
    callback : function(res) {
        if (io360.twitter.window) {
            io360.twitter.window.close();
            io360.twitter.window = null;
        }

        if (res) {
            io360.draw.alert("Success", "Your account has been linked with the twitter account: " + res);
            io360.auth.utwitter = res;
        } else {
            io360.draw.alert("Failure", "Your account has not been linked with twitter!");
            io360.auth.utwitter = false;
        }
        
        io360.auth.refreshView();
        io360.twitter.update();
    },
    
    tweet : function(link) {
        io360.twitter.show(link);
    },
    
    postTweet : function() {
        var tweet = $("#_tweet textarea.comment").val();
        
        if (!io360.auth.utwitter) {
            window.open('http://twitter.com/home?status=' + encodeURIComponent(tweet));
            return;
        }
        
        if (!io360.util.formsEnabled) return;
        io360.util.disableForms();
        
        io360.crossDomain.request(
                "http://service.360.io/crossdomain/bridge.html",
                "http://service.360.io/create/twitter/post", 
                {
                    tweet: tweet
                },
                function(data) {
                    io360.logging.log("io360.twitter.postTweet", data);
                    
                    data = io360.parse.data(data);
                    if (data && data.id) {
                        var link = "http://twitter.com/" + data.user.screen_name + "/status/" + data.id;
                        io360.draw.alert("Twitter", "Tweeted <a href='" + link + "' target='_blank'>here!</a>"); 
                    } else {
                        io360.draw.alert("Error", "Tweet Failed"); 
                    }
                    
                    io360.util.enableForms();
                });        
    }
    
}

io360.tooltip = {
    timeout : null,
    
    show : function(node, text) {
        var pos = $(node).offset();
        
        $("#_tooltip tr.r0 td.c0 div span").html(text);
        $("#_tooltip").css({
            left: (pos.left + $(node).width()/2 - $("#_tooltip").width()/2) + "px",
            top: (pos.top - 25) + "px"
        });
        
        if (io360.tooltip.timeout) {
            clearTimeout(io360.tooltip.timeout);
        }
        
        $("#_tooltip").show();
    },
    
    hide : function() {
        io360.tooltip.timeout = setTimeout(function() {
            $("#_tooltip").hide();
        }, 0);
    }
}

io360.dash = {
    toggle : function() {
        if (io360.floater.showing.dashboard) {
            io360.dash.close();
        } else {
            io360.dash.show();
        }
    },
    
    refresh : function() {
        io360.dash.close();
    },
    
    show : function() {
        if (io360.auth.uid) {
            io360.floater.show("dashboard");
        } else {
            io360.floater.show("auth");
        }
    },
    
    close : function() {
        io360.floater.hide("dashboard");
    }
}

io360.show = {
    display : function(body) {
        $("body").append($("<div>").click(function() {
            var el = this;
            $("#_overlay").fadeOut(125, function() {
                $(el).fadeOut(125, function() {
                    $("#_overlay").remove();
                    $(el).remove();
                });
            });
        }).css({
            'opacity': 0,
            'background': '#000000',
            'position': 'absolute',
            'top': 0,
            'left': 0,
            'right': 0,
            'bottom': 0,
            'zIndex': 100
        }).fadeTo(125, 0.50, function() {
            $("body").append($("<div>").css({
                'display': 'block',
                'position': 'absolute',
                'top': '50%',
                'left': '50%',
                'marginLeft': '-450px',
                'width': '900px',
                'zIndex': 100
            }).attr("id", "_overlay").html(body).css({
                'marginTop': (-1 * $("#_overlay").height() / 2) + 'px'
            }).append($("<div class='_shadow_t'>"))
                .append($("<div class='_shadow_tl'>"))
                .append($("<div class='_shadow_l'>"))
                .append($("<div class='_shadow_bl'>"))
                .append($("<div class='_shadow_b'>"))
                .append($("<div class='_shadow_br'>"))
                .append($("<div class='_shadow_r'>"))
                .append($("<div class='_shadow_tr'>")));

            var el = this;
            $("#_overlay").find("a").click(function() {
                $("#_overlay").fadeOut(125, function() {
                    $(el).fadeOut(125, function() {
                        $("#_overlay").remove();
                        $(el).remove();
                    });
                });                
            });
        }));
    }
}

io360.style = {
    current : "",
    previewSheet : null,
    fullSheet : null,
    
    safePrefix : function(id) {
        return "div._comment div.style_" + id + " ";
    },
    
    init : function() {
        document.getElementsByTagName("head")[0].appendChild(document.createElement("style"));
        io360.style.previewSheet = document.styleSheets[document.styleSheets.length - 1];
        
        document.getElementsByTagName("head")[0].appendChild(document.createElement("style"));
        io360.style.fullSheet = document.styleSheets[document.styleSheets.length - 1];
    },
    
    refresh : function() {
        $("#_stylemanager form").each(function() {
            $(this).addClass("_disabled");
            $("input", this).attr("disabled", "disabled");
            $("textarea", this).attr("disabled", "disabled");
        });
        
        io360.crossDomain.request(
                "http://service.360.io/crossdomain/bridge.html",
                "http://service.360.io/auth/style", 
                {
                },
                function(data) {
                    io360.logging.log("io360.style.init", data);
                    
                    data = io360.parse.data(data);
                    
                    var style = data.style;
                    var prefix = io360.style.safePrefix(io360.auth.uid);
                    
                    var index = style.indexOf(prefix);
                    while(index != -1){
                        style = style.replace(prefix, "");
                        index = style.indexOf(prefix);
                    }
                    
                    io360.style.current = style;
                    
                    io360.style.reset();
                    
                    $("#_stylemanager form").each(function() {
                        $(this).removeClass("_disabled");
                        $("input", this).attr("disabled", "");
                        $("textarea", this).attr("disabled", "");
                    });
                });
    },
    
    save : function() {
        if (!io360.util.formsEnabled || $("#_stylemanager form textarea").attr("disabled")) return;
        io360.util.disableForms();

        // add space here so that the argument makes it through to the parser in service node
        var style = " " + $("#_iostyle").val();

        io360.crossDomain.request(
                "http://service.360.io/crossdomain/bridge.html",
                "http://service.360.io/auth/style", 
                {
                    style : style
                },
                function(data) {
                    io360.logging.log("io360.style.save", data);
                    
                    io360.style.current = style;
                    
                    io360.util.enableForms();
                }, true);
    },
    
    reset : function() {
        $("#_iostyle").val(io360.style.current);
        io360.style.updatePreview();
    },
    
    updatePreview : function() {
        var i = 0;
        if (io360.style.previewSheet.rules) {
            while (io360.style.previewSheet.rules[i]) {
                io360.style.previewSheet.removeRule(i++);
            }
        } else {
            while (io360.style.previewSheet.cssRules[i]) {
                io360.style.previewSheet.deleteRule(i++);
            }
        }
        
        var prefix = "#_stylemanager div._preview div._render ";
        var style = $("#_iostyle").val();
        
        io360.style.process(prefix, style, io360.style.previewSheet);
    },
    
    process : function(prefix, style, sheet) {
        if (!style || !sheet) return;
        
        style = style.match(/([^,\{]*\{[^\}]*\})/g);
        for (var i = 0; i < style.length; i++) {
            if (style[i].substr(0, style[i].indexOf("{")).replace(/^\s+|\s+$/g,"").length == 0) continue;
            
            var rule = prefix + style[i];
            var selector = rule.substr(0, rule.indexOf("{")).replace(/^\s+|\s+$/, '');
            var ruleBody = rule.match(/\{([^\}]*)\}/)[1].replace(/^\s+|\s+$/, '');
            
            if (sheet.rules) {                
                sheet.addRule(selector, ruleBody);
            } else {
                sheet.insertRule(rule, sheet.cssRules.length);
            }
        }
    }
}