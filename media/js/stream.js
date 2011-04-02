io360.stream = {
    lid : null,
    arrivals : [],
    selected : null,
    
    publisherCommentPadding : 0,
    publisherInputsPadding : 0,
    
    showingComments : true,
    
    commentsByID : {},
    
    loadedHash : null,
    type : null,
    
    init : function(head, tail, sessionID) {
        io360.stream.type = head.type;
        io360.stream.lid = document.location.pathname.substr(1);
        io360.stream.loadedHash = document.location.hash;
        
        io360.engine.sessionID = sessionID;
        io360.core.init(io360.stream.postInit);
        
        head.head.user = head.user;
        io360.users.cache([head.head.user]);
        io360.stream.handleElement(head.head, true, true);        
        io360.stream.handleTail(tail, true);
        
        io360.listeners.broadcast = function(data) {
            io360.stream.handleTail(data);
        };
        
        $("a._btn_toggle").addClass("_btn_toggle_hide").removeClass("_btn_toggle_show");
        
        if (head.type != "text") {
            $("div._backclick").click(function() { io360.stream.toggle(); });
        } else {
            $("div._backclick").hide();
        }
        
        io360.commands.initCallback = function() {
            io360.commands.registerCommSession("tail_" + io360.stream.lid);
            io360.commands.authCommSession();
            io360.crossDomain.request(
                    "http://data.360.io/BRG/",
                    "http://data.360.io/TAIL/" + io360.stream.lid, 
                    {}, 
                    function(data) {
                        io360.logging.log("dataStream", data);

                        data = io360.parse.data(data);
                        io360.stream.handleTail(data);
                        io360.commands.connectCommSession();
                    });
        };
        
        if (head.type == "link") {
            document.title = "360io: " + head.head.params.param;
            document.getElementsByTagName('iframe')[0].src = head.head.params.param;
        } else {
            if (head.type == "pic") {
                var attached = head.head.params.attached;
                var first = null;
                var len = 0;
                var pics = "<div class='panel'>";
                
                for (var a in attached) {
                    if (!first) {
                        first = "<a href='http://360io.attachments.s3.amazonaws.com/" + attached[a].name + "' target='_blank'><img src='http://360io.attachments.s3.amazonaws.com/scaled_" + attached[a].name + "' /></a>";
                    }
                    
                    pics += "<a href='javascript:io360.stream.pic(\"" + attached[a].name + "\")'><img src='http://360io.attachments.s3.amazonaws.com/thumb_" + attached[a].name + "' /></a>";
                    len++;
                }
                
                pics += "</div>";
                if (len === 1) pics = "";
                pics += "<div class='view'>" + first + "</div>";
                pics += "<div class='comment'><h3>" + head.head.user.name + "</h3><p>" + head.head.body + "</p><a href='javascript:io360.publisher.toggle();'>Reply</a></div>";
                
                $("#_picframe").html(pics).show();
                $("#_activated").hide();
            } else {             
            }
        }
    },
    
    postInit : function(data) {
        $("#comment_null").after($("#_float_publisher"));
        io360.publisher.init();
        $(window).attr('scale')();
        
        if (io360.stream.loadedHash) {
            var id = io360.stream.loadedHash.substr(1);
            if (id && io360.stream.commentsByID[id]) {
                io360.stream.comment.show(id);
            }
        }
        
        if (io360.stream.type == 'pic') {
            io360.stream.close();
        } else if (io360.stream.type == 'text') {
            $("a._btn_toggle").hide();
            $("a._btn_post").css({
                marginLeft : (-1 * ($("a._btn_post").width() / 2))
            });
        }
    },
    
    close : function() {
        if (io360.stream.showingComments) {
            io360.stream.toggle();
        } else {
            window.location.href = document.getElementsByTagName('iframe')[0].src;
        }
    },
    
    handleTail : function(data, immediately) {
        io360.users.cache(data.users);
        
        var ids = [];
        var tail = data.tail;
        
        for (var i = 0; i < tail.length; i++) {
            var element = tail[i];
            io360.stream.handleElement(element, immediately);
            ids.push(element.id);
        }
        
        io360.stream.arrivals.push.apply(io360.stream.arrivals, ids);
        $("#_arrivals").html("<p>" + io360.stream.arrivals.length + " new comments have arrived. Click here to show them.</p>");
        $("#_arrivals").slideDown();
    },
    
    handleElement : function(element, immediately, hide) {
        if (io360.stream.commentsByID[element.id]) return;
        io360.stream.commentsByID[element.id] = element;
        
        var view = $(io360.stream.render.comment(element, immediately)).hover(
            function() { $(this).children("div._toolbar").show(); }, 
            function() { $(this).children("div._toolbar").hide(); }
        );
        
        var thread = element.id && element.id != "null" ? $("#thread_" + element.parent) : $("#_comments");
        thread.each(function() {
            var children = $(this).children('div._comment');
        
            var inserted = false;
            for (var n = 0; n < children.length; n++) {
                var child = $(children[n]);
                if (child.attr("time") < element.time) {
                    child.before(view);
                
                    inserted = true;
                    break;
                }
            }

            if (!inserted) {
                $(this).append(view);
            }
        });
        
        $(window).attr('scale')();
    },
    
    toggle : function() {
        if (io360.stream.showingComments) {
            $("div._backclick").hide();
            $("#_activated").fadeOut(125, function() {
                $("a._btn_toggle").addClass("_btn_toggle_show").removeClass("_btn_toggle_hide");
            });
        } else {
            $("div._backclick").show();
            $("#_activated").fadeIn(125, function() {
                io360.publisher.resize();
                $("a._btn_toggle").addClass("_btn_toggle_hide").removeClass("_btn_toggle_show");
            });
        }
        
        io360.stream.showingComments = !io360.stream.showingComments;
    },
    
    showArrivals : function() {
        var arrivals = io360.stream.arrivals;
        io360.stream.arrivals = [];
        
        for (var i = 0; i < arrivals.length; i++) {
            $("#comment_" + arrivals[i]).slideDown();
        }
        
        $("#_arrivals").slideUp();
    },
    
    pic : function(pic) {
        $("#_picframe div.view").html("<a href='http://360io.attachments.s3.amazonaws.com/" + pic + "' target='_blank'><img src='http://360io.attachments.s3.amazonaws.com/scaled_" + pic + "' /></a>");
    }
}

io360.publisher = {
    showing : true,
    height : 0,
    type : null,
    
    attachment : {
        values : {},
        
        onSelect : function(id, name, size) {
            $("<div class='_attachcontainer' id='_attach_" + id + "'>").append("<b>").append($("<div class='_attach'>").append($("<span class='_bar'>"))
                .append($("<span class='_label'>").html(name + " <i>(" + Math.round(size/1000) + "KB)</i>"))
                .append($("<a class='_cancel' href='javascript:io360.publisher.attachment.cancel(" + id + ");'>").html("Cancel")))
            .append("<b>")
            .insertBefore("#_aux b:first")

            io360.xdswf.transport.runUpload(id, "http://service.360.io/create/attachment", {});
            io360.publisher.initUpload();
            io360.publisher.resize();
        },
        
        onProgress : function(id, loaded, total) {
            $("#_attach_" + id + " span._bar").css({
                width: (loaded/total*100) + "%"
            });
        },
        
        onComplete : function(id, res) {
            io360.publisher.attachment.values[id] = res;
            $("#_attach_" + id + " span._bar").css({
                width: "100%"
            });
        },
        
        cancel : function(id) {
            io360.xdswf.transport.cancelUpload(id);
            delete io360.publisher.attachment.values[id];
            $("#_attach_" + id).remove();
        }
    },
    
    init : function() {
        $("#_float_publisher").children("h1").remove();
        $("#_float_publisher").children("a._btn_close").attr("onclick", "").click(function() {
            io360.publisher.close();
            return false;
        });
        
        $("#_aux div._attachwrapper").hide();
        $("#_aux div._inputwrapper").hide();
        $("div._attachwrapper").mouseover(function() {
            var target = $("._swftarget").removeClass("_ltebtnHover");
            io360.xdswf.swfdiv.css({
                position: 'absolute',
                top: $(target).offset().top - 1,
                left: $(target).offset().left - 1,
                width: $(target).width() + 2,
                height: $(target).height() + 2
            }).css('z-index', 10);
            return false;
        });
        
        $("#_float_publisher, div._backclick").mouseover(function() {
            $("._swftarget").removeClass("_ltebtnHover");
            io360.xdswf.swfdiv.css({
                position: 'absolute',
                top: 0,
                left: 0,
                width: 1,
                height: 1
            });            
        });
        
        io360.publisher.initUpload();
        io360.xdswf.swfdiv.hover(function(e) {
            $("._swftarget").addClass("_ltebtnHover");
        }, function(e) {
            $("._swftarget").removeClass("_ltebtnHover");
        });
        
        io360.publisher.select('text');
    },
    
    initUpload : function() {
        io360.xdswf.transport.initUpload("*.jpeg;*.jpg;*.png;*.gif", 
                                        "io360.publisher.attachment.onSelect", 
                                        "io360.publisher.attachment.onProgress",
                                        "io360.publisher.attachment.onComplete");
    },
    
    select : function(type) {
        io360.publisher.type = type;
        $("._publishSelected").removeClass("_publishSelected");
        $("._publish_" + type).addClass('_publishSelected');
        if (type == 'text') {
            $("#_aux").slideUp(75, function() {
                io360.publisher.resize(function() { 
                    if ($("#_activated").css("display") != "none") {
                        sliderAnimation.slideTo(Math.min(0, sliderHeight - 30 - ($("#_float_publisher").position().top + io360.publisher.height))); 
                    }
                });
            });
        } else {
            if (type == 'link') {
                $("#_aux div._attachcontainer").hide();
                $("#_aux div._attachwrapper").hide();
                $("#_aux div._inputwrapper").show();
                $("#_aux div._inputwrapper input").attr("default", "Enter a link to share").val("Enter a link to share");
            } else {
                $("#_aux div._attachcontainer").show();
                $("#_aux div._attachwrapper").show();
                $("#_aux div._inputwrapper").hide();
                $("#_aux div._attachwrapper a i").html("Attach and share an image");
            }
            
            $("#_aux").slideDown(75, function() {
                io360.publisher.resize(function() { 
                    sliderAnimation.slideTo(Math.min(0, sliderHeight - 30 - ($("#_float_publisher").position().top + io360.publisher.height))); 
                });
            });
        }
    },
    
    resize : function(after) {
        if ($("#_float_publisher div._internal").height()) {
            io360.publisher.height = $("#_float_publisher div._internal").height() + 25;
        }
        
        $("#_float_publisher").animate({
            opacity : 1,
            height : io360.publisher.height + 'px'
        }, 125, 'linear', function() {
            $(window).attr('scale')();
            if (after) after();
        });
    },
    
    info : function() {
        io360.draw.alert("Hello", "I really am that sexy");
    },
    
    toggle : function() {
        if (!io360.stream.showingComments) {
            io360.stream.toggle();   
        }
        
        io360.publisher.show();
    },
    
    show : function() {
        io360.publisher.resize();
        if (io360.stream.comment.selected) {
            $("#_float_publisher").animate({
                opacity : 0
            }, 125, 'linear', function() {
                io360.stream.comment.selected = null;
                $("#comment_null").after($("#_float_publisher"));
                io360.publisher.show();
            });
        } else {
            io360.publisher.showing = true;
            $("#_float_publisher").show();
            $("#_float_publisher").animate({
                opacity : 1,
                height : io360.publisher.height + 'px'
            }, 125, 'linear', function() {
                $(window).attr('scale')();
                sliderAnimation.slideTo(Math.min(0, sliderHeight - 30 - ($("#_float_publisher").position().top + io360.publisher.height)));
            });
        }
    },

    close : function() {
        io360.publisher.showing = false;
        $("#_float_publisher").animate({
            opacity : 0
        }, 125, 'linear', function() {
            $("#_float_publisher").hide().css('height', '0px');
            $(window).attr('scale')();
        });
    },
    
    post : function() {
        var publisher = $("#_publisher form");
        var name = publisher.find("div._inputs div._inputwrapper input.name").val();
        var email = publisher.find("div._inputs div._inputwrapper input.email").val();
        var body = publisher.find("div._textwrapper textarea.body").val();
        
        if (body == "" || body == publisher.find("div._textwrapper textarea.body").attr("default")) {
            io360.draw.alert("Error", "Enter a comment first!");
            return;
        }
        
        if (name == "" || name == publisher.find("div._inputs div._inputwrapper input.name").attr("default")) {
            name = "anonymous";
        }
        
        if (!io360.util.formsEnabled) return;
        io360.util.disableForms();
        
        io360.crossDomain.request(
                "http://service.360.io/crossdomain/bridge.html",
                "http://service.360.io/create/tail", 
                {
                    type : 'comment',
                    lid : io360.stream.lid,
                    pid : io360.stream.comment.selected,
                    uid : $("#comment_" + io360.stream.comment.selected).attr("uid"),
                    name : name,
                    email : email,
                    type : io360.publisher.type,
                    param : io360.publisher.type == 'link' ? $("input.aux").val() : '',
                    body : body,
                    attached : io360.publisher.type == 'pic' ? io360.xdswf.uneval(io360.publisher.attachment.values) : ''
                },
                function(data) {
                    io360.logging.log("io360.stream.submit", data);
                    
                    publisher.find("div._textwrapper textarea.body").val("Write your comment here...");
                    io360.publisher.close();
                    
                    $("input.aux").val($("input.aux").attr("default"));
                    for (var a in io360.publisher.attachment.values) {
                        delete io360.publisher.attachment.values[a];
                        $("#_attach_" + a).remove();
                    }
                    
                    io360.publisher.height = $("#_float_publisher div._internal").height() + 25;
                    io360.util.enableForms();
                });        
    },
    
    auxAttachment : function() {
    }
}

io360.users = {
    cache : function(users) {
        for (var i = 0; i < users.length; i++) {
            if (users[i] && users[i].id && !io360.users[users[i].id]) {
                io360.users[users[i].id] = users[i];
                io360.style.process("", users[i].style, io360.style.fullSheet);
            }
        }
    }
}

io360.stream.render = {    
    comment : function(comment, immediately) {
        comment.id = comment.id || null;
        
        var user = io360.users[comment.uid] || io360.parse.data(comment.user);
        var link = "http://360.io/" + io360.stream.lid + "#" + comment.id;
        var img = "<img src='http://www.gravatar.com/avatar/" + (io360.users[comment.uid] ? user.id : '3b3be63a4c2a439b013787725dfce802') + "?s=50' />";
        var drawer = '';
        var toolstyle = '';
        
        if (comment.type == 'pic' && comment.params.attached) {
            drawer += "<div class='_drawer'><div class='bg'></div><div class='slide'>";
            
            for (var a in comment.params.attached) {
                drawer += "<a href='http://360io.attachments.s3.amazonaws.com/" + comment.params.attached[a].name + "' target='_blank'><img src='http://360io.attachments.s3.amazonaws.com/thumb_" + comment.params.attached[a].name + "' /></a>";
            }
            
            drawer += "</div></div>";
            toolstyle = 'style="bottom: 50px;"';
        } else if (comment.type == 'link' && comment.params.param) {
            drawer += "<div class='_linkpanel'><div class='bg'></div><div class='slide'>";
            drawer += "<a href='" + comment.params.param + "' target='_blank'>" + comment.params.param + "</a>";
            drawer += "</div></div>";
            toolstyle = 'style="bottom: 30px;"';            
        }
        
        var view = '<div id="comment_' + comment.id + '" time="' + comment.time + '" uid="' + comment.uid + '" class="_comment">';
        view += '<div class="_shadow_t"></div><div class="_shadow_tr"></div><div class="_shadow_r"></div><div class="_shadow_br"></div>';
        view += '<div class="_shadow_b"></div><div class="_shadow_bl"></div><div class="_shadow_l"></div><div class="_shadow_tl"></div>';
        view += '<div class="_styled style_' + comment.uid + '"><div class="comment"><div class="header"><div class="avatar">' + img + '</div>';
        view += '<div class="author">' + user.name + '</div><div class="meta">' + comment.time + '</div></div>';
        view += '<div class="body">' + comment.body + '</div></div></div>';
        view += '<div class="_toolbar" ' + toolstyle + '>';
        view += '<a href="javascript:io360.stream.comment.reply(\'' + comment.id + '\')"" class="_reply">Reply</a>';
        view += '<a href="javascript:io360.stream.comment.tweet(\'' + comment.id + '\')"" class="_tweet">Tweet</a></div>';
        view += drawer;
        view += '</div><div id="thread_' + comment.id + '" class="_thread"></div>';

        if (!immediately) {
            io360.draw.notification("Comment", user.name + " posted a commented", function() { io360.stream.comment.show(comment.id); });
        }
        
        return view;
    }
}

io360.stream.comment = {
    selected : null,

    show : function(id) {
        if (!io360.stream.showingComments) {
            io360.stream.toggle();   
        }
        
        sliderAnimation.slideTo(Math.min(0, Math.min($("#comment_" + id).position().top, $("#_slid").height() - sliderHeight - 30) * -1));
    },

    reply : function(id) {
        io360.publisher.resize();
        if (!io360.stream.showingComments) {
            io360.stream.toggle();   
        }
        
        if (id == 'null') id = null;

        if (io360.stream.comment.selected != id && io360.publisher.showing) {
            $("#_float_publisher").animate({
                opacity : 0
                //height : (io360.publisher.height / 2) + 'px'
            }, 125, 'linear', function() {
                io360.stream.comment.selected = id;
                //sliderAnimation.slideTo(Math.min($("#comment_" + id).position().top, $("#_slid").height() + $("#_float_publisher").height() - $("#_slider").height()) * -1);
                io360.stream.comment.reply(id);
            });
        } else {
            io360.stream.comment.selected = id;        
            io360.publisher.showing = true;
            $("#comment_" + id).after($("#_float_publisher"));
            $("#_float_publisher").show();
            $("#_float_publisher").animate({
                opacity : 1,
                height : io360.publisher.height + 'px'
            }, 125, 'linear', function() {
                $(window).attr('scale')();
                sliderAnimation.slideTo(Math.min(0, sliderHeight - 30 - ($("#_float_publisher").position().top + io360.publisher.height)));
            });
        }
    },
    
    tweet : function(id) {
        if (io360.stream.lid) {
            var link = "http://360.io/" + io360.stream.lid;
        
            if (id != "null") {
                link += "#" + id;
            }
        
            io360.twitter.tweet(link);
        }
    },
}